//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2006 Ulrich von Zadow
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  Current versions can be found at www.libavg.de
//
// V4L2/libavg compliance by 02L > Outside Standing Level

#include "V4LCamera.h"

#include "../base/ScopeTimer.h"
#include "../base/TimeSource.h"
#include "../base/Logger.h"

#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>

#include <linux/videodev2.h>

#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>
#include <iostream>

#define CLEAR(x) memset (&(x), 0, sizeof (x))

using namespace avg;
using namespace std;

// anonymous namespace holding private (C-static-like) functions
namespace {
    int xioctl (int Fd, int Request, void * Arg)
    {
        int Rc;

        do {
            Rc = ioctl(Fd, Request, Arg);
        } while (Rc == -1 && EINTR == errno);

        return Rc;
    }
}

V4LCamera::V4LCamera(std::string sDevice, int Channel, IntPoint Size,
        const std::string &PixelFormat, bool bColor)
           : m_Fd(-1),
           m_Channel(Channel),
           m_sDevice(sDevice), 
           m_bCameraAvailable(false),
           m_bColor(bColor),
           m_ImgSize(Size)
{
    AVG_TRACE(Logger::CONFIG, "V4LCamera() device=" << sDevice << 
        " ch=" << Channel << " w=" << Size.x << " h=" << Size.y <<
        " pf=" << PixelFormat);
    
    m_CamPF = getCamPF(PixelFormat);

    m_FeaturesNames[V4L2_CID_BRIGHTNESS] = "brightness";
    m_FeaturesNames[V4L2_CID_CONTRAST] = "contrast";
    m_FeaturesNames[V4L2_CID_GAIN] = "gain";
    m_FeaturesNames[V4L2_CID_EXPOSURE] = "exposure";
    m_FeaturesNames[V4L2_CID_WHITENESS] = "whiteness";
    m_FeaturesNames[V4L2_CID_GAMMA] = "gamma";
    m_FeaturesNames[V4L2_CID_SATURATION] = "saturation";
}

V4LCamera::~V4LCamera() 
{
    close();
}

void V4LCamera::open()
{
    struct stat St; 

    if ( stat(m_sDevice.c_str(), &St) == -1) {
        AVG_TRACE(Logger::ERROR, "Unable to access v4l device " << 
          m_sDevice);
        // TODO: Disable camera instead of exit(1).
        exit(1);
    }

    if (!S_ISCHR (St.st_mode)) {
        AVG_TRACE(Logger::ERROR, m_sDevice + " is not a v4l device");
        // TODO: Disable camera instead of exit(1).
        exit(1);
    }

    m_Fd = ::open(m_sDevice.c_str(), O_RDWR /* required */ | O_NONBLOCK, 0);

    if (m_Fd == -1) {
        AVG_TRACE(Logger::ERROR, "Unable to open v4l device " << m_sDevice);
        // TODO: Disable camera instead of exit(1).
        exit(1);
    }
    
    initDevice();
    startCapture();
    AVG_TRACE(Logger::CONFIG, "V4L Camera opened");
}

void V4LCamera::close()
{
    if (m_bCameraAvailable) {
        enum v4l2_buf_type Type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        if (-1 == xioctl (m_Fd, VIDIOC_STREAMOFF, &Type)) {
            AVG_TRACE(Logger::ERROR, "VIDIOC_STREAMOFF");
        }
        vector<Buffer>::iterator it;
        for (it=m_vBuffers.begin(); it != m_vBuffers.end(); ++it) {
            if (-1 == munmap (it->start, it->length)) {
                AVG_TRACE(Logger::WARNING, "V4lCamera::close(): munmap failed.");
            }
        }
        m_vBuffers.clear();

        if ( ::close(m_Fd) == -1) {
            AVG_TRACE(Logger::ERROR, "Error on closing v4l device");
        }
        AVG_TRACE(Logger::CONFIG, "V4L Camera closed");

        m_Fd = -1;
        m_bCameraAvailable = false;
    }
}

IntPoint V4LCamera::getImgSize()
{
    return m_ImgSize;
}

int V4LCamera::getCamPF(const std::string& sPF)
{
    int pfDef = V4L2_PIX_FMT_BGR24;

    if (sPF == "MONO8") {
        pfDef = V4L2_PIX_FMT_GREY;
    }
    /*
    // NOT SUPPORTED YET
    else if (sPF == "YUV411") {
        pfDef = V4L2_PIX_FMT_Y41P;
    }*/
    else if (sPF == "YUV422") {
        pfDef = V4L2_PIX_FMT_UYVY;
    }
    else if (sPF == "YUYV422") {
        pfDef = V4L2_PIX_FMT_YUYV;
    }
    else if (sPF == "YUV420") {
        pfDef = V4L2_PIX_FMT_YUV420;
    }
    else if (sPF == "RGB") {
        pfDef = V4L2_PIX_FMT_BGR24;
    }
    else {
        AVG_TRACE (Logger::WARNING,
                std::string("Unsupported or illegal value for camera \
                pixel format \"") 
                + sPF + std::string("\"."));
    }
    
    return pfDef;
}


static ProfilingZone CameraConvertProfilingZone("V4L Camera format conversion");

BitmapPtr V4LCamera::getImage(bool bWait)
{
    unsigned char *pSrc = 0;
    
    struct v4l2_buffer Buf;
    CLEAR(Buf);
    
    // wait for incoming data blocking, timeout 2s
    if (bWait) {
        fd_set Fds;
        struct timeval Tv;
        int Rc;
    
        FD_ZERO (&Fds);
        FD_SET (m_Fd, &Fds);
    
        /* Timeout. */
        Tv.tv_sec = 2;
        Tv.tv_usec = 0;
    
        Rc = select (m_Fd + 1, &Fds, NULL, NULL, &Tv);

        // caught signal or something else  
        if (Rc == -1) {
            AVG_TRACE(Logger::WARNING, "V4L: select failed.");
            return BitmapPtr();
        }
        // timeout
        if (Rc == 0) {
            AVG_TRACE(Logger::WARNING, "V4L: Timeout while waiting \
               for image data");
            return BitmapPtr();
        }
    }
    
    Buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    Buf.memory = V4L2_MEMORY_MMAP;
    
    // dequeue filled buffer
    if (xioctl (m_Fd, VIDIOC_DQBUF, &Buf) == -1) {
        switch (errno) {
            case EAGAIN:
                    return BitmapPtr();

            case EIO:
                    AVG_TRACE(Logger::ERROR, "EIO");
                    exit(1);
                    break;

            case EINVAL:
                    AVG_TRACE(Logger::ERROR, "EINVAL");
                    exit(1);
                    break;

            default:
                    AVG_TRACE(Logger::ERROR, "VIDIOC_DQBUF");
                    exit(1);
        }
    }
    
    pSrc = (unsigned char*)m_vBuffers[Buf.index].start;
        
    IntPoint Size = getImgSize();

    // target bitmap and uchar* to
    BitmapPtr pCurBitmap;
    if (m_bColor) {
        pCurBitmap = BitmapPtr(new Bitmap(Size, B8G8R8X8));
    }
    else {
        pCurBitmap = BitmapPtr(new Bitmap(Size, I8));
    }
    
    unsigned char *pDest = pCurBitmap->getPixels();

    ScopeTimer Timer(CameraConvertProfilingZone);

    switch (m_CamPF) {
    // fast BGR24 -> B8G8R8X8 conversion
    case V4L2_PIX_FMT_BGR24:
        for (int imgp = 0 ; imgp < Size.x * Size.y ; ++imgp) {
            *pDest++ = *pSrc++; // Blue
            *pDest++ = *pSrc++; // Green
            *pDest++ = *pSrc++; // Red
            *pDest++ = 0xFF;    // Alpha
        }
        break;
    case V4L2_PIX_FMT_GREY:
        {
            Bitmap TempBmp(Size, I8, pSrc, Size.x, false, 
                    "TempCameraBmp");
            pCurBitmap->copyPixels(TempBmp);
        }
        break;
    case V4L2_PIX_FMT_UYVY:
        {
            Bitmap TempBmp(Size, YCbCr422, pSrc, Size.x*2, false, 
                    "TempCameraBmp");
            pCurBitmap->copyPixels(TempBmp);
        }
        break;
    case V4L2_PIX_FMT_YUYV:
        {
            Bitmap TempBmp(Size, YUYV422, pSrc, Size.x*2, false, 
                    "TempCameraBmp");
            pCurBitmap->copyPixels(TempBmp);
        }
        break;
    case V4L2_PIX_FMT_YUV420:
        {
            Bitmap TempBmp(Size, YCbCr420p, pSrc, Size.x, false, 
                    "TempCameraBmp");
            pCurBitmap->copyPixels(TempBmp);
        }
        break;
    /*
    // NOT SUPPORTED YET
    case V4L2_PIX_FMT_Y41P:
    {
        Bitmap TempBmp(Size, YCbCr411, pSrc, (int)(Size.x*1.5), false, "TempCameraBmp");
        pCurBitmap->copyPixels(TempBmp);
    }
    break; */
    }

    // enqueues free buffer for mmap
    if (-1 == xioctl (m_Fd, VIDIOC_QBUF, &Buf)) {
        AVG_TRACE(Logger::ERROR, "VIDIOC_DQBUF");
        exit(1);
    }
    
    return pCurBitmap;
}

bool V4LCamera::isCameraAvailable()
{
    return m_bCameraAvailable;
}

const std::string& V4LCamera::getDevice() const
{
    return m_sDevice;
}

const std::string& V4LCamera::getDriverName() const
{
    return m_sDriverName;
}

double V4LCamera::getFrameRate() const
{
    // TODO: PAL or NTSC have different frame rates
    return 25;
}

std::string V4LCamera::getFeatureName(V4LCID_t V4LFeature)
{
    std::string sName;

    sName = m_FeaturesNames[V4LFeature];
    
    if (sName == "") {
        sName = "UNKNOWN";
    }
    
    return sName;
}

V4LCID_t V4LCamera::getFeatureID(const std::string& sFeature) const
{
    V4LCID_t V4LFeature;
    if (sFeature == "brightness") {
        V4LFeature = V4L2_CID_BRIGHTNESS;
    } else if (sFeature == "contrast") {
        V4LFeature = V4L2_CID_CONTRAST;
    }  else if (sFeature == "gain") {
        V4LFeature = V4L2_CID_GAIN;
    } else if (sFeature == "exposure") {
        V4LFeature = V4L2_CID_EXPOSURE;
    } else if (sFeature == "whiteness") {
        V4LFeature = V4L2_CID_WHITENESS;
    } else if (sFeature == "gamma") {
        V4LFeature = V4L2_CID_GAMMA;
    } else if (sFeature == "saturation") {
        V4LFeature = V4L2_CID_SATURATION;
    } else {
        AVG_TRACE(Logger::WARNING, "Unknown feature " << sFeature);
        return -1;
    }
    
    return V4LFeature;
}

bool V4LCamera::isFeatureSupported(V4LCID_t V4LFeature) const
{
    struct v4l2_queryctrl QueryCtrl;
    
    CLEAR(QueryCtrl);
    QueryCtrl.id = V4LFeature;
    
    if (ioctl (m_Fd, VIDIOC_QUERYCTRL, &QueryCtrl) == -1) {
            if (errno != EINVAL) {
                    AVG_TRACE(Logger::ERROR,"VIDIOC_QUERYCTRL");
                    exit(1);
            } 
            else {
                return false;
            }
    } else if (QueryCtrl.flags & V4L2_CTRL_FLAG_DISABLED) {
        return false;
    } else {
        return true;
    }
}

unsigned int V4LCamera::getFeature(const std::string& sFeature) const
{
    V4LCID_t V4LFeature = getFeatureID(sFeature);
    
    FeatureMap::const_iterator it = m_Features.find(V4LFeature);
    
    if (it == m_Features.end()) {
        return 0;
    } else {
        return it->second;
    }
}

void V4LCamera::setFeature(V4LCID_t V4LFeature, int Value)
{
    if (!m_bCameraAvailable) {
        AVG_TRACE(Logger::WARNING, "setFeature() called before opening \
            device: ignored");
        return;
    }

    if (!isFeatureSupported(V4LFeature)) {
        AVG_TRACE(Logger::WARNING, "Feature " << getFeatureName(V4LFeature) <<
            " is not supported by hardware");
        return;
    }
    
    struct v4l2_control control;
    
    CLEAR(control);
    control.id = V4LFeature;
    control.value = Value;

//    AVG_TRACE(Logger::APP, "Setting Feature " << getFeatureName(V4LFeature) << 
//      " to "<< Value);

    if (ioctl (m_Fd, VIDIOC_S_CTRL, &control) == -1) {
        AVG_TRACE(Logger::ERROR, "Cannot set feature " <<
            m_FeaturesNames[V4LFeature]);
    }
}

void V4LCamera::setFeature(const std::string& sFeature, int Value)
{
    // ignore -1 coming from default unbiased cameranode parameters
    if (Value < 0) return;
    
    V4LCID_t V4LFeature = getFeatureID(sFeature);

    m_Features[V4LFeature] = Value;

//    AVG_TRACE(Logger::WARNING,"Setting feature " << sFeature <<
//        " to " << Value);
    if (m_bCameraAvailable) {
        setFeature(V4LFeature, Value);
    }
}

void V4LCamera::startCapture()
{
//  AVG_TRACE(Logger::APP, "Entering startCapture()...");

    unsigned int i;
    enum v4l2_buf_type Type;

    for (i = 0; i < m_vBuffers.size(); ++i) {
        struct v4l2_buffer Buf;

        CLEAR (Buf);

        Buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        Buf.memory      = V4L2_MEMORY_MMAP;
        Buf.index       = i;

        if (-1 == xioctl (m_Fd, VIDIOC_QBUF, &Buf)) {
            AVG_TRACE(Logger::ERROR, "VIDIOC_QBUF");
            exit(1);
        }
    }
    
    Type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    if (-1 == xioctl (m_Fd, VIDIOC_STREAMON, &Type)) {
        AVG_TRACE(Logger::ERROR, "VIDIOC_STREAMON");
        exit(1);
    }

}

void V4LCamera::initDevice()
{
//  AVG_TRACE(Logger::APP, "Entering initDevice()...");
    
    struct v4l2_capability Cap;
    struct v4l2_cropcap CropCap;
    struct v4l2_crop Crop;
    struct v4l2_format Fmt;

    if (-1 == xioctl (m_Fd, VIDIOC_QUERYCAP, &Cap)) {
        if (EINVAL == errno) {
            AVG_TRACE(Logger::ERROR, m_sDevice << " is not a valid \
                V4L2 device");
            exit(1);
        } else {
            AVG_TRACE(Logger::ERROR, "VIDIOC_QUERYCAP error");
            exit(1);
        }
    }

    if (!(Cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
        AVG_TRACE(Logger::ERROR, m_sDevice << " does not support \
            capturing");
        exit(1);
    }

    if (!(Cap.capabilities & V4L2_CAP_STREAMING)) {
        AVG_TRACE(Logger::ERROR, m_sDevice << " does not support \
            streaming i/os");
        exit(1);
    }
    m_sDriverName = (const char *)Cap.driver;

    /* Select video input, video standard and tune here. */
    CLEAR (CropCap);
    CropCap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    if (xioctl(m_Fd, VIDIOC_CROPCAP, &CropCap) == 0) {
        Crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        Crop.c = CropCap.defrect; /* reset to default */

        if (-1 == xioctl (m_Fd, VIDIOC_S_CROP, &Crop)) {
            switch (errno) {
                case EINVAL:
                    /* Cropping not supported. */
                    break;
                default:
                    /* Errors ignored. */
                    break;
            }
        }
    } else {   
            /* Errors ignored. */
    }

    CLEAR (Fmt);

    Fmt.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    Fmt.fmt.pix.width       = getImgSize().x; 
    Fmt.fmt.pix.height      = getImgSize().y;
    Fmt.fmt.pix.pixelformat = m_CamPF;
    Fmt.fmt.pix.field       = V4L2_FIELD_ANY;

    if (xioctl(m_Fd, VIDIOC_S_FMT, &Fmt) == -1) {
        AVG_TRACE(Logger::ERROR, m_sDevice << " could not set image format (" 
                << strerror(errno) << ")");
        close();
        exit(1);
    }

    initMMap ();
    
    // TODO: string channel instead of numeric
    // select channel
    if (xioctl(m_Fd, VIDIOC_S_INPUT, &m_Channel) == -1) {
        AVG_TRACE(Logger::ERROR, "Cannot set MUX channel " << m_Channel);
        close();
        exit(1);
    }
    
    m_bCameraAvailable = true;

    for (FeatureMap::iterator it=m_Features.begin(); it != m_Features.end(); it++) {
        setFeature(it->first, it->second);
    }

    
}

void V4LCamera::initMMap()
{
    struct v4l2_requestbuffers Req;
    CLEAR (Req);

    Req.count = 4;
    Req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    Req.memory = V4L2_MEMORY_MMAP;

    if (xioctl (m_Fd, VIDIOC_REQBUFS, &Req) == -1) {
        if (EINVAL == errno) {
            AVG_TRACE(Logger::ERROR, m_sDevice << " does not support \
                memory mapping");
            exit(1);
        } else {
            AVG_TRACE(Logger::ERROR, "V4LCamera::initMMap: " << strerror(errno));
            exit(1);
        }
    }
    
    if (Req.count < 2) {
        AVG_TRACE(Logger::ERROR, "Insufficient buffer memory on " << m_sDevice);
        exit(1);
    }

    m_vBuffers.clear();

    for (int i=0; i < int(Req.count); ++i) {
        Buffer Tmp;
        struct v4l2_buffer Buf;

        CLEAR (Buf);

        Buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        Buf.memory = V4L2_MEMORY_MMAP;
        Buf.index = i;

        if (xioctl (m_Fd, VIDIOC_QUERYBUF, &Buf) == -1) {
            AVG_TRACE(Logger::ERROR, "VIDIOC_QUERYBUF index=" << i);
            exit(1);
        }

        Tmp.length = Buf.length;
        
        Tmp.start = mmap (NULL /* start anywhere */,
            Buf.length,
            PROT_READ | PROT_WRITE /* required */,
            MAP_SHARED /* recommended */,
            m_Fd, Buf.m.offset);

        if (MAP_FAILED == Tmp.start) {
            AVG_TRACE(Logger::ERROR, "mmap() failed on buffer index=" << i);
            exit(1);
        }
                
        m_vBuffers.push_back(Tmp);
    }
}



