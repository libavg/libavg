//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2008 Ulrich von Zadow
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
//  V4L2/libavg compliance by 02L > Outside Standing Level

#include "V4LCamera.h"

#include "../base/ScopeTimer.h"
#include "../base/TimeSource.h"
#include "../base/Logger.h"
#include "../base/Exception.h"
#include "../base/StringHelper.h"

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
#include <cstring>

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
        PixelFormat camPF, PixelFormat destPF)
    : Camera(camPF, destPF),
      m_Fd(-1),
      m_Channel(Channel),
      m_sDevice(sDevice), 
      m_ImgSize(Size)
{
    m_v4lPF = getV4LPF(camPF);
    if (m_sDevice == "") {
        m_sDevice = "/dev/video0";
    }

    m_FeaturesNames[V4L2_CID_BRIGHTNESS] = "brightness";
    m_FeaturesNames[V4L2_CID_CONTRAST] = "contrast";
    m_FeaturesNames[V4L2_CID_GAIN] = "gain";
    m_FeaturesNames[V4L2_CID_EXPOSURE] = "exposure";
    m_FeaturesNames[V4L2_CID_WHITENESS] = "whiteness";
    m_FeaturesNames[V4L2_CID_GAMMA] = "gamma";
    m_FeaturesNames[V4L2_CID_SATURATION] = "saturation";
    
    struct stat St; 
    if (stat(m_sDevice.c_str(), &St) == -1) {
        fatalError(string("Unable to access v4l2 device '")+m_sDevice+"'." );
    }

    if (!S_ISCHR (St.st_mode)) {
        fatalError(string("'")+m_sDevice+" is not a v4l2 device.");
    }

    m_Fd = ::open(m_sDevice.c_str(), O_RDWR /* required */ | O_NONBLOCK, 0);

    if (m_Fd == -1) {
        fatalError(string("Unable to open v4l2 device '") + m_sDevice + "'.");
    }
    
    initDevice();
    startCapture();
    AVG_TRACE(Logger::CONFIG, "V4L2 Camera opened");
}

V4LCamera::~V4LCamera() 
{
    close();
}


void V4LCamera::close()
{
    enum v4l2_buf_type Type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (-1 == xioctl (m_Fd, VIDIOC_STREAMOFF, &Type)) {
        AVG_TRACE(Logger::ERROR, "VIDIOC_STREAMOFF");
    }
    vector<Buffer>::iterator it;
    for (it=m_vBuffers.begin(); it != m_vBuffers.end(); ++it) {
        int err = munmap (it->start, it->length);
        assert (err != -1);
    }
    m_vBuffers.clear();

    ::close(m_Fd);
    AVG_TRACE(Logger::CONFIG, "V4L2 Camera closed");

    m_Fd = -1;
}

IntPoint V4LCamera::getImgSize()
{
    return m_ImgSize;
}

int V4LCamera::getV4LPF(PixelFormat pf)
{
    switch(pf) {
        case I8:
            return V4L2_PIX_FMT_GREY;
        case YCbCr411:
            return V4L2_PIX_FMT_Y41P;
        case YCbCr422:
            return V4L2_PIX_FMT_UYVY;
        case YUYV422:
            return V4L2_PIX_FMT_YUYV;
        case YCbCr420p:
            return V4L2_PIX_FMT_YUV420;
        case R8G8B8:
            return V4L2_PIX_FMT_BGR24;
        default:
            throw Exception(AVG_ERR_INVALID_ARGS,
                    "Unsupported or illegal value for camera pixel format '" 
                + Bitmap::getPixelFormatString(pf) + "'.");
    }
}

BitmapPtr V4LCamera::getImage(bool bWait)
{
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
            AVG_TRACE(Logger::WARNING, "V4L2: select failed.");
            return BitmapPtr();
        }
        // timeout
        if (Rc == 0) {
            AVG_TRACE(Logger::WARNING, "V4L2: Timeout while waiting for image data");
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
                throw(Exception(AVG_ERR_CAMERA, "Got EIO when acquiring image."));
            case EINVAL:
                throw(Exception(AVG_ERR_CAMERA, "Got EINVAL when acquiring image."));
            default:
                throw(Exception(AVG_ERR_CAMERA, "Unknown error acquiring image."));
        }
    }
    
    unsigned char * pCaptureBuffer = (unsigned char*)m_vBuffers[Buf.index].start;
   
    IntPoint size = getImgSize();
    double lineLen;
    switch (getCamPF()) {
        case YCbCr411:
            lineLen = size.x*1.5;
            break;
        case YCbCr420p:
            lineLen = size.x;
            break;
        default:
            lineLen = size.x*Bitmap::getBytesPerPixel(getCamPF());
    }
    BitmapPtr pCamBmp(new Bitmap(size, getCamPF(), pCaptureBuffer, lineLen,
            false, "TempCameraBmp"));

    BitmapPtr pDestBmp = convertCamFrameToDestPF(pCamBmp);

    // enqueues free buffer for mmap
    if (-1 == xioctl (m_Fd, VIDIOC_QBUF, &Buf)) {
        throw(Exception(AVG_ERR_CAMERA, "V4L Camera: failed to enqueue image buffer."));
    }
    
    return pDestBmp;
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
    // TODO: PAL or NTSC have different frame rates. So do USB cameras!
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

V4LCID_t V4LCamera::getFeatureID(CameraFeature Feature) const
{
    V4LCID_t V4LFeature;
    if (Feature == CAM_FEATURE_BRIGHTNESS) {
        V4LFeature = V4L2_CID_BRIGHTNESS;
    } else if (Feature == CAM_FEATURE_CONTRAST) {
        V4LFeature = V4L2_CID_CONTRAST;
    }  else if (Feature == CAM_FEATURE_GAIN) {
        V4LFeature = V4L2_CID_GAIN;
    } else if (Feature == CAM_FEATURE_EXPOSURE) {
        V4LFeature = V4L2_CID_EXPOSURE;
    } else if (Feature == CAM_FEATURE_GAMMA) {
        V4LFeature = V4L2_CID_GAMMA;
    } else if (Feature == CAM_FEATURE_SATURATION) {
        V4LFeature = V4L2_CID_SATURATION;
    } else {
        AVG_TRACE(Logger::WARNING, "Feature " << cameraFeatureToString(Feature)
                << " not supported for V4L2.");
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
                cerr << "Got " << strerror(errno) << endl;
                assert(false);
            } else {
                return false;
            }
    } else if (QueryCtrl.flags & V4L2_CTRL_FLAG_DISABLED) {
        return false;
    } else {
        return true;
    }
}

int V4LCamera::getFeature(CameraFeature Feature) const
{
    V4LCID_t V4LFeature = getFeatureID(Feature);
    
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
        AVG_TRACE(Logger::WARNING, "setFeature() called before opening device: ignored");
        return;
    }

    if (!isFeatureSupported(V4LFeature)) {
        AVG_TRACE(Logger::WARNING, "Camera feature " << getFeatureName(V4LFeature) <<
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

void V4LCamera::setFeatureOneShot(CameraFeature Feature)
{
    AVG_TRACE(Logger::WARNING, "setFeatureOneShot is not supported by V4L cameras.");
}

int V4LCamera::getWhitebalanceU() const
{
    AVG_TRACE(Logger::WARNING, "getWhitebalance is not supported by V4L cameras.");
    return 0;
}

int V4LCamera::getWhitebalanceV() const
{
    AVG_TRACE(Logger::WARNING, "getWhitebalance is not supported by V4L cameras.");
    return 0;
}

void V4LCamera::setWhitebalance(int u, int v, bool bIgnoreOldValue)
{
    setFeature(V4L2_CID_RED_BALANCE, u); 
    setFeature(V4L2_CID_BLUE_BALANCE, v); 
}

void V4LCamera::setFeature(CameraFeature Feature, int Value, bool bIgnoreOldValue)
{
    // ignore -1 coming from default unbiased cameranode parameters
    if (Value < 0) return;
    
    V4LCID_t V4LFeature = getFeatureID(Feature);

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

        int err = xioctl (m_Fd, VIDIOC_QBUF, &Buf);
        assert(err != -1);
    }
    
    Type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    int err= xioctl (m_Fd, VIDIOC_STREAMON, &Type);
    assert(err != -1);
}

void V4LCamera::initDevice()
{
//  AVG_TRACE(Logger::APP, "Entering initDevice()...");
    
    struct v4l2_capability Cap;
    struct v4l2_cropcap CropCap;
    struct v4l2_crop Crop;
    struct v4l2_format Fmt;

    if (xioctl(m_Fd, VIDIOC_QUERYCAP, &Cap) == -1) {
        close();
        fatalError(m_sDevice + " is not a valid V4L2 device.");
    }

    if (!(Cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
        close();
        fatalError(m_sDevice + " does not support capturing");
    }

    if (!(Cap.capabilities & V4L2_CAP_STREAMING)) {
        close();
        fatalError(m_sDevice + " does not support streaming i/os");
    }
    m_sDriverName = (const char *)Cap.driver;

    /* Select video input, video standard and tune here. */
    CLEAR(CropCap);
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
    Fmt.fmt.pix.pixelformat = m_v4lPF;
    Fmt.fmt.pix.field       = V4L2_FIELD_ANY;

    if (xioctl(m_Fd, VIDIOC_S_FMT, &Fmt) == -1) {
        close();
        throw(Exception(AVG_ERR_CAMERA, string("Unable to set V4L camera image format: '")
                +strerror(errno)+"'. Try using v4l_info to find out what the device supports."));
    }

    initMMap ();
    
    // TODO: string channel instead of numeric
    // select channel
    if (xioctl(m_Fd, VIDIOC_S_INPUT, &m_Channel) == -1) {
        close();
        fatalError(string("Cannot set MUX channel ")+toString(m_Channel));
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
            close();
            fatalError(m_sDevice+" does not support memory mapping");
        } else {
            cerr << "errno: " << strerror(errno);
            assert(false);
        }
    }
    
    if (Req.count < 2) {
        cerr << "Insufficient buffer memory on " << m_sDevice;
        assert(false);
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
            assert(false);
        }

        Tmp.length = Buf.length;
        
        Tmp.start = mmap (NULL /* start anywhere */,
            Buf.length,
            PROT_READ | PROT_WRITE /* required */,
            MAP_SHARED /* recommended */,
            m_Fd, Buf.m.offset);

        if (MAP_FAILED == Tmp.start) {
            assert(false);
        }
                
        m_vBuffers.push_back(Tmp);
    }
}



