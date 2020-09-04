//
//  libavg - Media Playback Engine.
//  Copyright (C) 2003-2020 Ulrich von Zadow
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
#include "../base/GLMHelper.h"

#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>

#include <linux/videodev2.h>
#include <jpeglib.h>

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
    int xioctl(int fd, int request, void * arg)
    {
        int rc;

        do {
            rc = ioctl(fd, request, arg);
        } while (rc == -1 && EINTR == errno);

        return rc;
    }
}

namespace avg {

V4LCamera::V4LCamera(string sDevice, int channel, IntPoint size, PixelFormat camPF,
        PixelFormat destPF, float frameRate)
    : Camera(camPF, destPF, size, frameRate),
      m_Fd(-1),
      m_Channel(channel),
      m_sDevice(sDevice)
{
    m_v4lPF = getV4LPF(camPF);
    if (m_sDevice == "") {
        m_sDevice = "/dev/video0";
    }
    if (m_Channel == -1) {
        m_Channel = 0;
    }

    m_FeaturesNames[V4L2_CID_BRIGHTNESS] = "brightness";
    m_FeaturesNames[V4L2_CID_CONTRAST] = "contrast";
    m_FeaturesNames[V4L2_CID_GAIN] = "gain";
    m_FeaturesNames[V4L2_CID_EXPOSURE] = "exposure";
    m_FeaturesNames[V4L2_CID_WHITENESS] = "whiteness";
    m_FeaturesNames[V4L2_CID_GAMMA] = "gamma";
    m_FeaturesNames[V4L2_CID_SATURATION] = "saturation";

    struct stat st;
    if (stat(m_sDevice.c_str(), &st) == -1) {
        AVG_ASSERT_MSG(false, (string("Unable to access v4l2 device '" +
                m_sDevice + "'.").c_str()));
    }

    if (!S_ISCHR (st.st_mode)) {
        AVG_ASSERT_MSG(false, (string("'" + m_sDevice +
                " is not a v4l2 device.").c_str()));
    }

    m_Fd = ::open(m_sDevice.c_str(), O_RDWR /* required */ | O_NONBLOCK, 0);

    if (m_Fd == -1) {
        AVG_ASSERT_MSG(false, (string("Unable to open v4l2 device '" + m_sDevice
                + "'.").c_str()));
    }

    initDevice();
    AVG_TRACE(Logger::category::CONFIG, Logger::severity::INFO, "V4L2 Camera opened");
}

V4LCamera::~V4LCamera()
{
    close();
}

void V4LCamera::close()
{
    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    int rc = xioctl(m_Fd, VIDIOC_STREAMOFF, &type);
    if (rc == -1) {
        AVG_LOG_ERROR("VIDIOC_STREAMOFF");
    }
    vector<Buffer>::iterator it;
    for (it = m_vBuffers.begin(); it != m_vBuffers.end(); ++it) {
        int err = munmap(it->start, it->length);
        AVG_ASSERT (err != -1);
    }
    m_vBuffers.clear();

    ::close(m_Fd);
    AVG_TRACE(Logger::category::CONFIG, Logger::severity::INFO, "V4L2 Camera closed");

    m_Fd = -1;
}

int V4LCamera::getV4LPF(PixelFormat pf)
{
    switch (pf) {
        case I8:
            return V4L2_PIX_FMT_GREY;
        case BAYER8:
        case BAYER8_BGGR:
        case BAYER8_GBRG:
        case BAYER8_GRBG:
        case BAYER8_RGGB:
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
        case JPEG:
            return V4L2_PIX_FMT_MJPEG;
        default:
            throw Exception(AVG_ERR_INVALID_ARGS,
                    "Unsupported or illegal value for camera pixel format '"
                + getPixelFormatString(pf) + "'.");
    }
}

BitmapPtr V4LCamera::getImage(bool bWait)
{
    struct v4l2_buffer buf;
    CLEAR(buf);

    // wait for incoming data blocking, timeout 2s
    if (bWait) {
        fd_set fds;
        struct timeval tv;
        int rc;

        FD_ZERO(&fds);
        FD_SET(m_Fd, &fds);

        /* Timeout. */
        tv.tv_sec = 2;
        tv.tv_usec = 0;

        rc = select (m_Fd+1, &fds, NULL, NULL, &tv);

        // caught signal or something else
        if (rc == -1) {
            AVG_LOG_WARNING("V4L2: select failed.");
            return BitmapPtr();
        }
        // timeout
        if (rc == 0) {
            AVG_LOG_WARNING("V4L2: Timeout while waiting for image data");
            return BitmapPtr();
        }
    }

    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;

    // dequeue filled buffer
    if (xioctl (m_Fd, VIDIOC_DQBUF, &buf) == -1) {
        if (errno == EAGAIN) {
            return BitmapPtr();
        } else {
            cerr << strerror(errno) << endl;
            AVG_ASSERT(false);
        }
    }

    unsigned char * pCaptureBuffer = (unsigned char*)m_vBuffers[buf.index].start;

    BitmapPtr pDestBmp;
    if (getCamPF() == JPEG) {
        pDestBmp = decompressJpegFrame(pCaptureBuffer);
    } else {
        float lineLen;
        switch (getCamPF()) {
            case YCbCr411:
                lineLen = getImgSize().x*1.5f;
                break;
            case YCbCr420p:
                lineLen = getImgSize().x;
                break;
            default:
                lineLen = getImgSize().x*getBytesPerPixel(getCamPF());
        }
        BitmapPtr pCamBmp = BitmapPtr(new Bitmap(getImgSize(), getCamPF(),
                pCaptureBuffer, lineLen, false, "TempCameraBmp"));
        pDestBmp = convertCamFrameToDestPF(pCamBmp);
//        cerr << "CamBmp: " << pCamBmp->getPixelFormat() << ", DestBmp: "
//                << pDestBmp->getPixelFormat() << endl;
    }

    // enqueues free buffer for mmap
    if (-1 == xioctl (m_Fd, VIDIOC_QBUF, &buf)) {
        AVG_ASSERT_MSG(false, "V4L Camera: failed to enqueue image buffer.");
    }

    return pDestBmp;
}

bool V4LCamera::isCameraAvailable()
{
    return m_bCameraAvailable;
}

const string& V4LCamera::getDevice() const
{
    return m_sDevice;
}

const string& V4LCamera::getDriverName() const
{
    return m_sDriverName;
}

string V4LCamera::getFeatureName(V4LCID_t v4lFeature)
{
    string sName = m_FeaturesNames[v4lFeature];
    if (sName == "") {
        sName = "UNKNOWN";
    }

    return sName;
}

V4LCID_t V4LCamera::getFeatureID(CameraFeature feature) const
{
    V4LCID_t v4lFeature;
    if (feature == CAM_FEATURE_BRIGHTNESS) {
        v4lFeature = V4L2_CID_BRIGHTNESS;
    } else if (feature == CAM_FEATURE_CONTRAST) {
        v4lFeature = V4L2_CID_CONTRAST;
    }  else if (feature == CAM_FEATURE_GAIN) {
        v4lFeature = V4L2_CID_GAIN;
    } else if (feature == CAM_FEATURE_EXPOSURE) {
        v4lFeature = V4L2_CID_EXPOSURE;
    } else if (feature == CAM_FEATURE_GAMMA) {
        v4lFeature = V4L2_CID_GAMMA;
    } else if (feature == CAM_FEATURE_SATURATION) {
        v4lFeature = V4L2_CID_SATURATION;
    } else {
        AVG_LOG_WARNING("feature " << cameraFeatureToString(feature)
                << " not supported for V4L2.");
        return -1;
    }

    return v4lFeature;
}

bool V4LCamera::isFeatureSupported(V4LCID_t v4lFeature) const
{
    struct v4l2_queryctrl queryCtrl;

    CLEAR(queryCtrl);
    queryCtrl.id = v4lFeature;

    if (ioctl (m_Fd, VIDIOC_QUERYCTRL, &queryCtrl) == -1) {
        if (errno != EINVAL) {
            cerr << "Got " << strerror(errno) << endl;
            AVG_ASSERT(false);
            return false;
        } else {
            return false;
        }
    } else if (queryCtrl.flags & V4L2_CTRL_FLAG_DISABLED) {
        return false;
    } else {
        return true;
    }
}

int V4LCamera::getFeature(CameraFeature feature) const
{
    V4LCID_t v4lFeature = getFeatureID(feature);

    FeatureMap::const_iterator it = m_Features.find(v4lFeature);

    if (it == m_Features.end()) {
        return 0;
    } else {
        return it->second;
    }
}

void V4LCamera::setFeature(V4LCID_t v4lFeature, int value)
{
    if (!m_bCameraAvailable) {
        AVG_LOG_WARNING("setFeature() called before opening device: ignored");
        return;
    }

    if (!isFeatureSupported(v4lFeature)) {
        AVG_LOG_WARNING("Camera feature " << getFeatureName(v4lFeature) <<
            " is not supported by hardware");
        return;
    }

    struct v4l2_control control;

    CLEAR(control);
    control.id = v4lFeature;
    control.value = value;

//    AVG_TRACE(Logger::category::APP, "Setting feature " << getFeatureName(v4lFeature) <<
//      " to "<< value);

    if (ioctl(m_Fd, VIDIOC_S_CTRL, &control) == -1) {
        AVG_LOG_ERROR("Cannot set feature " << m_FeaturesNames[v4lFeature]);
    }
}

void V4LCamera::setFeatureOneShot(CameraFeature feature)
{
    AVG_LOG_WARNING("setFeatureOneShot is not supported by V4L cameras.");
}

int V4LCamera::getWhitebalanceU() const
{
    AVG_LOG_WARNING("getWhitebalance is not supported by V4L cameras.");
    return 0;
}

int V4LCamera::getWhitebalanceV() const
{
    AVG_LOG_WARNING("getWhitebalance is not supported by V4L cameras.");
    return 0;
}

void V4LCamera::setWhitebalance(int u, int v, bool bIgnoreOldValue)
{
    setFeature(V4L2_CID_RED_BALANCE, u);
    setFeature(V4L2_CID_BLUE_BALANCE, v);
}

int V4LCamera::checkCamera(int j)
{
    stringstream minorDeviceNumber;
    minorDeviceNumber << j;
    string address = "/dev/video";
    string result = address + minorDeviceNumber.str();
    int fd = ::open(result.c_str(), O_RDWR /* required */ | O_NONBLOCK, 0);
    return fd;
}

v4l2_capability getCamCapabilities(int fd)
{
    v4l2_capability capability;
    memset(&capability, 0, sizeof(capability));
    ioctl(fd, VIDIOC_QUERYCAP, &capability);
    return capability;
}

PixelFormat V4LCamera::intToPixelFormat(unsigned int pixelformat)
{
    switch (pixelformat) {
        case v4l2_fourcc('Y','U','Y','V'):
            return YUYV422;
        case v4l2_fourcc('U','Y','V','Y'):
            return YCbCr422;
        case v4l2_fourcc('G','R','E','Y'):
            return I8;
        case v4l2_fourcc('Y','1','6',' '):
            return I16;
        case v4l2_fourcc('R','G','B','3'):
            return R8G8B8;
        case v4l2_fourcc('B','G','R','3'):
            return B8G8R8;
        case v4l2_fourcc('M','J','P','G'):
        case v4l2_fourcc('J','P','E','G'):
            return JPEG;
        default:
            return NO_PIXELFORMAT;
    }
}

int V4LCamera::countCameras()
{
    int numberOfCameras = 0;
    for (int j = 0; j < 256; j++) {
        int fd = checkCamera(j);
        if (fd != -1) {
            numberOfCameras++;
        }
    }
    return numberOfCameras;
}

CameraInfo* V4LCamera::getCameraInfos(int deviceNumber)
{
    int fd = checkCamera(deviceNumber);
    if (fd == -1) {
        AVG_ASSERT(false);
        return NULL;
    }
    stringstream ss;
    ss << "/dev/video" << deviceNumber;
    CameraInfo* camInfo = new CameraInfo("video4linux", ss.str());
    v4l2_capability capability = getCamCapabilities(fd);
    if (capability.capabilities & V4L2_CAP_VIDEO_CAPTURE) {
        getCameraImageFormats(fd, camInfo);
        getCameraControls(fd, camInfo);
    }
    return camInfo;
}

void V4LCamera::getCameraImageFormats(int fd, CameraInfo* camInfo)
{
    for (int i = 0;; i++) {
//        cerr << i << endl;
        v4l2_fmtdesc fmtDesc;
        memset(&fmtDesc, 0, sizeof(fmtDesc));
        fmtDesc.index = i;
        fmtDesc.type  = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        int rc = ioctl(fd, VIDIOC_ENUM_FMT, &fmtDesc);
        if (rc == -1) {
            break;
        }
        v4l2_frmsizeenum frmSizeEnum;
        memset(&frmSizeEnum, 0, sizeof (frmSizeEnum));
        frmSizeEnum.index = 0;
        frmSizeEnum.pixel_format = fmtDesc.pixelformat;
        while (ioctl(fd, VIDIOC_ENUM_FRAMESIZES, &frmSizeEnum) == 0) {
/*            fprintf(stdout, "  pixelformat  :%c%c%c%c\n",
                                fmtDesc.pixelformat & 0xFF,
                                (fmtDesc.pixelformat >> 8) & 0xFF,
                                (fmtDesc.pixelformat >> 16) & 0xFF, 
                                (fmtDesc.pixelformat >> 24) & 0xFF);
*/
            PixelFormat pixFormat = intToPixelFormat(fmtDesc.pixelformat);
            if (pixFormat != NO_PIXELFORMAT) {
                v4l2_frmivalenum frmIvalEnum;
                memset(&frmIvalEnum, 0, sizeof (frmIvalEnum));
                frmIvalEnum.index = 0;
                frmIvalEnum.pixel_format = frmSizeEnum.pixel_format;
                frmIvalEnum.width = frmSizeEnum.discrete.width;
                frmIvalEnum.height = frmSizeEnum.discrete.height;
                IntPoint size;
                size.x = frmSizeEnum.discrete.width;
                size.y = frmSizeEnum.discrete.height;
                std::vector<float> framerates;
                while (ioctl(fd, VIDIOC_ENUM_FRAMEINTERVALS, &frmIvalEnum) == 0) {
                    float framerate = (float)frmIvalEnum.discrete.denominator
                                    / (float)frmIvalEnum.discrete.numerator;
/*                    fprintf(stdout, "    framerate  :(%d, %d) -> %f\n",
                            frmIvalEnum.discrete.denominator,
                            frmIvalEnum.discrete.numerator, framerate);
*/
                    framerates.push_back(framerate);
                    frmIvalEnum.index++;
                }
                CameraImageFormat camImFormat = CameraImageFormat(size, pixFormat,
                        framerates);
                camInfo->addImageFormat(camImFormat);
            }
            frmSizeEnum.index++;
        }
    }
}

void V4LCamera::getCameraControls(int fd, CameraInfo* camInfo)
{
    v4l2_queryctrl queryCtrl;
    for (queryCtrl.id = V4L2_CID_BASE; queryCtrl.id < V4L2_CID_LASTP1; queryCtrl.id++) {
        int rc = ioctl (fd, VIDIOC_QUERYCTRL, &queryCtrl);
        if (rc == -1) {
            continue;
        }
        if (queryCtrl.flags & V4L2_CTRL_FLAG_DISABLED) {
            continue;
        }
        stringstream ss;
        ss << queryCtrl.name;
        std::string sControlName = ss.str();
        int min = queryCtrl.minimum;
        int max = queryCtrl.maximum;
        int defaultValue = queryCtrl.default_value;
        CameraControl camControl = CameraControl(sControlName, min, max, defaultValue);
        camInfo->addControl(camControl);
    }
}

void V4LCamera::setFeature(CameraFeature feature, int value, bool bIgnoreOldValue)
{
    // ignore -1 coming from default unbiased cameranode parameters
    if (value < 0) {
        return;
    }

    V4LCID_t v4lFeature = getFeatureID(feature);
    m_Features[v4lFeature] = value;

    if (m_bCameraAvailable) {
        setFeature(v4lFeature, value);
    }
}

void V4LCamera::startCapture()
{
//  AVG_TRACE(Logger::category::APP, "Entering startCapture()...");

    unsigned int i;
    enum v4l2_buf_type type;

    for (i = 0; i < m_vBuffers.size(); ++i) {
        struct v4l2_buffer buf;

        CLEAR(buf);

        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;

        int err = xioctl(m_Fd, VIDIOC_QBUF, &buf);
        AVG_ASSERT(err != -1);
    }

    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    int err= xioctl (m_Fd, VIDIOC_STREAMON, &type);
    AVG_ASSERT(err != -1);

    if (getCamPF() == BAYER8) {
        if (m_sModelName == "DFx 31AU03") {
            AVG_TRACE(Logger::category::CONFIG, Logger::severity::INFO,
                    "Applying bayer pattern fixup for IS DFx31AU03 camera");
            setCamPF(BAYER8_GRBG);
        }
    }
}

void V4LCamera::initDevice()
{
//  AVG_TRACE(Logger::category::APP, "Entering initDevice()...");

    struct v4l2_capability cap;
    struct v4l2_cropcap CropCap;
    struct v4l2_crop Crop;
    struct v4l2_format fmt;
    struct v4l2_streamparm StreamParam;

    if (xioctl(m_Fd, VIDIOC_QUERYCAP, &cap) == -1) {
        close();
        AVG_ASSERT_MSG(false, (m_sDevice + " is not a valid V4L2 device.").c_str());
    }

    if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
        close();
        AVG_ASSERT_MSG(false, (m_sDevice + " does not support capturing").c_str());
    }

    if (!(cap.capabilities & V4L2_CAP_STREAMING)) {
        close();
        AVG_ASSERT_MSG(false, (m_sDevice + " does not support streaming i/os").c_str());
    }
    m_sDriverName = (const char *)cap.driver;
    m_sModelName = (const char *)cap.card;

    /* Select video input, video standard and tune here. */
    CLEAR(CropCap);
    CropCap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    if (xioctl(m_Fd, VIDIOC_CROPCAP, &CropCap) == 0) {
        Crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        Crop.c = CropCap.defrect; /* reset to default */

        if (-1 == xioctl(m_Fd, VIDIOC_S_CROP, &Crop)) {
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

    CLEAR(fmt);

    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width = getImgSize().x;
    fmt.fmt.pix.height = getImgSize().y;
    fmt.fmt.pix.pixelformat = m_v4lPF;
    fmt.fmt.pix.field = V4L2_FIELD_ANY;
    int rc = xioctl(m_Fd, VIDIOC_S_FMT, &fmt);
    if (int(fmt.fmt.pix.width) != getImgSize().x ||
            int(fmt.fmt.pix.height) != getImgSize().y || 
            fmt.fmt.pix.pixelformat != m_v4lPF ||
            rc == -1)
    {
        throw(Exception(AVG_ERR_CAMERA_NONFATAL,
                string("Unable to set V4L camera image format: '")
                +strerror(errno)
                +"'. Try using avg_showcamera.py --list to find out what the device supports."));
    }

    CLEAR(StreamParam);
    StreamParam.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    rc = xioctl(m_Fd, VIDIOC_G_PARM, &StreamParam);

    if(StreamParam.parm.capture.capability == V4L2_CAP_TIMEPERFRAME) {
        CLEAR(StreamParam);

        StreamParam.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        StreamParam.parm.capture.timeperframe.numerator = 1;
        StreamParam.parm.capture.timeperframe.denominator = (int)getFrameRate();
        rc = xioctl(m_Fd, VIDIOC_S_PARM, &StreamParam);
/*        fprintf(stdout, "framerate=%f, denominator=%d, numerator=%d\n",
                getFrameRate(),
                StreamParam.parm.capture.timeperframe.denominator,
                StreamParam.parm.capture.timeperframe.numerator);
*/
        float framerate = (float)StreamParam.parm.capture.timeperframe.denominator
                        / (float)StreamParam.parm.capture.timeperframe.numerator;
        if (getFrameRate() != framerate || rc == -1) {
            throw(Exception(AVG_ERR_CAMERA_NONFATAL,
                        string("Unable to set V4L camera framerate: '")
                        +strerror(errno)
                        +"'. Try using avg_showcamera.py --list to find out what the device supports."));
        }
    }

    initMMap();

    // TODO: string channel instead of numeric
    // select channel
    if (xioctl(m_Fd, VIDIOC_S_INPUT, &m_Channel) == -1) {
        close();
        AVG_ASSERT_MSG(false, (string("Cannot set MUX channel " +
                toString(m_Channel))).c_str());
    }

    m_bCameraAvailable = true;

    for (FeatureMap::iterator it=m_Features.begin(); it != m_Features.end(); it++) {
        setFeature(it->first, it->second);
    }
}

void V4LCamera::initMMap()
{
    struct v4l2_requestbuffers req;
    CLEAR(req);

    req.count = 4;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;

    if (xioctl(m_Fd, VIDIOC_REQBUFS, &req) == -1) {
        if (EINVAL == errno) {
            close();
            AVG_ASSERT_MSG(false, (m_sDevice +
                    " does not support memory mapping").c_str());
        } else {
            cerr << "errno: " << strerror(errno);
            AVG_ASSERT(false);
        }
    }

    if (req.count < 2) {
        cerr << "Insufficient buffer memory on " << m_sDevice;
        AVG_ASSERT(false);
    }

    m_vBuffers.clear();

    for (int i = 0; i < int(req.count); ++i) {
        Buffer tmp;
        struct v4l2_buffer buf;

        CLEAR (buf);

        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;

        if (xioctl(m_Fd, VIDIOC_QUERYBUF, &buf) == -1) {
            AVG_ASSERT(false);
        }

        tmp.length = buf.length;

        tmp.start = mmap (NULL /* start anywhere */,
            buf.length,
            PROT_READ | PROT_WRITE /* required */,
            MAP_SHARED /* recommended */,
            m_Fd, buf.m.offset);

        if (MAP_FAILED == tmp.start) {
            AVG_ASSERT(false);
        }

        m_vBuffers.push_back(tmp);
    }
}

// ISO/IEC 10918-1:1993(E) K.3.3. Default Huffman tables used by JPEG devices
// which don't specify a Huffman table in the JPEG stream.
// Tables and table copy code borrowed from libuvc (https://github.com/ktossell/libuvc).

static const unsigned char dc_lumi_len[] =
    {0, 0, 1, 5, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0};
static const unsigned char dc_lumi_val[] =
    {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};

static const unsigned char dc_chromi_len[] =
    {0, 0, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0};
static const unsigned char dc_chromi_val[] =
    {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};

static const unsigned char ac_lumi_len[] =
    {0, 0, 2, 1, 3, 3, 2, 4, 3, 5, 5, 4, 4, 0, 0, 1, 0x7d};
static const unsigned char ac_lumi_val[] =
    {0x01, 0x02, 0x03, 0x00, 0x04, 0x11, 0x05, 0x12, 0x21,
     0x31, 0x41, 0x06, 0x13, 0x51, 0x61, 0x07, 0x22, 0x71,
     0x14, 0x32, 0x81, 0x91, 0xa1, 0x08, 0x23, 0x42, 0xb1,
     0xc1, 0x15, 0x52, 0xd1, 0xf0, 0x24, 0x33, 0x62, 0x72,
     0x82, 0x09, 0x0a, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x25,
     0x26, 0x27, 0x28, 0x29, 0x2a, 0x34, 0x35, 0x36, 0x37,
     0x38, 0x39, 0x3a, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48,
     0x49, 0x4a, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59,
     0x5a, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a,
     0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x83,
     0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8a, 0x92, 0x93,
     0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9a, 0xa2, 0xa3,
     0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xb2, 0xb3,
     0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xc2, 0xc3,
     0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xd2, 0xd3,
     0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xe1, 0xe2,
     0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea, 0xf1,
     0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa};

static const unsigned char ac_chromi_len[] =
    {0, 0, 2, 1, 2, 4, 4, 3, 4, 7, 5, 4, 4, 0, 1, 2, 0x77};
static const unsigned char ac_chromi_val[] =
    {0x00, 0x01, 0x02, 0x03, 0x11, 0x04, 0x05, 0x21, 0x31,
     0x06, 0x12, 0x41, 0x51, 0x07, 0x61, 0x71, 0x13, 0x22,
     0x32, 0x81, 0x08, 0x14, 0x42, 0x91, 0xa1, 0xb1, 0xc1,
     0x09, 0x23, 0x33, 0x52, 0xf0, 0x15, 0x62, 0x72, 0xd1,
     0x0a, 0x16, 0x24, 0x34, 0xe1, 0x25, 0xf1, 0x17, 0x18,
     0x19, 0x1a, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x35, 0x36,
     0x37, 0x38, 0x39, 0x3a, 0x43, 0x44, 0x45, 0x46, 0x47,
     0x48, 0x49, 0x4a, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58,
     0x59, 0x5a, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69,
     0x6a, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a,
     0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8a,
     0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9a,
     0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xaa,
     0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba,
     0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca,
     0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda,
     0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea,
     0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa};

#define COPY_HUFF_TABLE(dinfo,tbl,name) do { \
    if (dinfo->tbl == NULL) dinfo->tbl = jpeg_alloc_huff_table((j_common_ptr)dinfo); \
    memcpy(dinfo->tbl->bits, name##_len, sizeof(name##_len)); \
    memset(dinfo->tbl->huffval, 0, sizeof(dinfo->tbl->huffval)); \
    memcpy(dinfo->tbl->huffval, name##_val, sizeof(name##_val)); \
} while(0)

static void insert_huff_tables(j_decompress_ptr dinfo) {
    COPY_HUFF_TABLE(dinfo, dc_huff_tbl_ptrs[0], dc_lumi);
    COPY_HUFF_TABLE(dinfo, dc_huff_tbl_ptrs[1], dc_chromi);
    COPY_HUFF_TABLE(dinfo, ac_huff_tbl_ptrs[0], ac_lumi);
    COPY_HUFF_TABLE(dinfo, ac_huff_tbl_ptrs[1], ac_chromi);
}

BitmapPtr V4LCamera::decompressJpegFrame(unsigned char* pCaptureBuffer)
{
    struct jpeg_decompress_struct dinfo;
    struct jpeg_error_mgr err;
    jpeg_create_decompress(&dinfo);
    dinfo.err = jpeg_std_error(&err);

    jpeg_mem_src(&dinfo, pCaptureBuffer, getImgSize().x * getImgSize().y);
    jpeg_read_header(&dinfo, true);

    if (dinfo.dc_huff_tbl_ptrs[0] == NULL) {
        // this frame is missing the Huffman tables, fill in the standard ones
        insert_huff_tables(&dinfo);
    }
    dinfo.out_color_space = getDestPF() == B8G8R8X8 ? JCS_EXT_BGRX : JCS_EXT_RGBX;
    dinfo.dct_method = JDCT_IFAST;

    BitmapPtr pDestBmp = BitmapPtr(new Bitmap(getImgSize(), getDestPF()));
    unsigned char* pPixels = pDestBmp->getPixels();

    jpeg_start_decompress(&dinfo);
    while (dinfo.output_scanline < dinfo.output_height) {
        int numScanlines = jpeg_read_scanlines(&dinfo, &pPixels, 1);
        pPixels += numScanlines * dinfo.output_width * dinfo.output_components;
    }
    jpeg_finish_decompress(&dinfo);
    jpeg_destroy_decompress(&dinfo);

    return pDestBmp;
}

}

