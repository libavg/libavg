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

#define CLEAR(x) memset (&(x), 0, sizeof (x))

// TODO:
// . setFeature() should map requests on a feature list and apply
//		them all only when camera becomes available

using namespace avg;

// anonymous namespace holding private (C-static-like) functions
namespace {
	int xioctl (int fd, int request, void * arg)
	{
        int r;

        do r = ioctl (fd, request, arg);
        while (r == -1 && EINTR == errno);

        return r;
	}
}

V4LCamera::V4LCamera(std::string sDevice, int Channel, std::string sMode, 
	bool bColor)
	: fd_(-1), Channel_(Channel), m_sDevice(sDevice), 
	  ioMethod_(V4LCamera::IO_METHOD_MMAP),
	  m_sMode(sMode),
	  m_bCameraAvailable(false),
	  m_bColor(bColor)
    /* 
      m_FrameRate(FrameRate)
	*/
{
	AVG_TRACE(Logger::APP, "V4LCamera() sMode=" << m_sMode);
	
	
	// MODE STRING TOKENIZATION (size / pixelformat)
	std::vector<std::string> tokens;
	std::string delimiter = "_";

	// Skip delimiters at beginning.
	std::string::size_type lastPos = m_sMode.find_first_not_of(delimiter, 0);
	// Find first "non-delimiter".
	std::string::size_type pos     = m_sMode.find_first_of(delimiter, lastPos);

	while (std::string::npos != pos || std::string::npos != lastPos)
	{
		// Found a token, add it to the vector.
		tokens.push_back(m_sMode.substr(lastPos, pos - lastPos));
		// Skip delimiters.  Note the "not_of"
		lastPos = m_sMode.find_first_not_of(delimiter, pos);
		// Find next "non-delimiter"
		pos = m_sMode.find_first_of(delimiter, lastPos);
	}

	if (tokens.size() == 2)
	{
		AVG_TRACE(Logger::APP, "Mode tokens: " << tokens[0] << " / " << tokens[1]);
		
		m_CamPF = getCamPF(tokens[1]);
	}
	else
	{
		AVG_TRACE(Logger::WARNING, "Unable to parse pixelformat, defaulting to RGB");
		
		m_CamPF = getCamPF("RGB");
	}

	// TODO: crop size tokenization
}

V4LCamera::~V4LCamera() 
{
}

void V4LCamera::open()
{
	struct stat st; 

    if ( stat(m_sDevice.c_str(), &st) == -1)
    {
    	AVG_TRACE(Logger::ERROR, "Unable to open v4l device " << m_sDevice);
        // TODO: Disable camera instead of exit(-1).
        exit(-1);
    }

    if (!S_ISCHR (st.st_mode))
    {
    	AVG_TRACE(Logger::ERROR, m_sDevice + " is not a v4l device");
        // TODO: Disable camera instead of exit(-1).
        exit(-1);
    }

    fd_ = ::open(m_sDevice.c_str(), O_RDWR /* required */ | O_NONBLOCK, 0);

    if (fd_ == -1)
    {
    	AVG_TRACE(Logger::ERROR, "Unable to open v4l device " << m_sDevice);
        // TODO: Disable camera instead of exit(-1).
        exit(-1);
    }
    
//    AVG_TRACE(Logger::APP, "Device opened, calling initDevice()...");
    
    initDevice();
    startCapture();
}

void V4LCamera::close()
{
	AVG_TRACE(Logger::APP, "Closing Camera...");
	if ( ::close(fd_) == -1)
        AVG_TRACE(Logger::ERROR, "Error on closing v4l device");

    fd_ = -1;
    m_bCameraAvailable = false;
}

IntPoint V4LCamera::getImgSize()
{
	// TODO: dynamic crop size definition
	return IntPoint(640, 480);
}

int V4LCamera::getCamPF(const std::string& sPF)
{
	int pfDef = V4L2_PIX_FMT_BGR24;

	if (sPF == "MONO8")
	{
		AVG_TRACE(Logger::APP, "Selecting grayscale I8 pixel format");
        pfDef = V4L2_PIX_FMT_GREY;
    }
    /*
    // NOT SUPPORTED YET
    else if (sPF == "YUV411")
    {
		AVG_TRACE(Logger::APP, "Selecting YUV4:1:1 pixel format");
		pfDef = V4L2_PIX_FMT_Y41P;
    }*/
    else if (sPF == "YUV422")
    {
		AVG_TRACE(Logger::APP, "Selecting YUV4:2:2 pixel format");
		pfDef = V4L2_PIX_FMT_UYVY;
    }
    else if (sPF == "YUV420")
    {
		AVG_TRACE(Logger::APP, "Selecting YUV4:2:0 pixel format");
		pfDef = V4L2_PIX_FMT_YUV420;
    }
    else if (sPF == "RGB")
    {
		AVG_TRACE(Logger::APP, "Selecting RGB (BGR24) pixel format");
		pfDef = V4L2_PIX_FMT_BGR24;
    }
    else
    {
        AVG_TRACE (Logger::WARNING,
                std::string("Unsupported or illegal value for camera pixel format \"") 
                + sPF + std::string("\"."));
    }
    
    return pfDef;
}


static ProfilingZone CameraConvertProfilingZone("      Camera format conversion");

BitmapPtr V4LCamera::getImage(bool bWait)
{
//	AVG_TRACE(Logger::APP, "getImage()");
	unsigned char *pSrc = 0;
	
	struct v4l2_buffer buf;
	CLEAR(buf);
	
	// wait for incoming data blocking, timeout 2s
	if (bWait)
	{
	    fd_set fds;
	    struct timeval tv;
	    int r;
	
	    FD_ZERO (&fds);
	    FD_SET (fd_, &fds);
	
	    /* Timeout. */
	    tv.tv_sec = 2;
	    tv.tv_usec = 0;
	
	    r = select (fd_ + 1, &fds, NULL, NULL, &tv);

		// caught signal or something else	
	    if (r == -1) return BitmapPtr();
	    // timeout
	    if (0 == r) {
	    	AVG_TRACE(Logger::WARNING, "V4L: Timeout while waiting for image data");
	        return BitmapPtr();
	    }
	}
	
	switch (ioMethod_)
	{
	case IO_METHOD_READ:
		if (read (fd_, buffers_[0].start, buffers_[0].length) == -1)
		{
//			AVG_TRACE(Logger::WARNING, "Frame not ready");
			return BitmapPtr();
		}
//		else AVG_TRACE(Logger::WARNING, "Frame read");

		pSrc = (unsigned char *)(buffers_[0].start);

		break;

	case IO_METHOD_MMAP:
		
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;
		
		// dequeue filled buffer
		if (-1 == xioctl (fd_, VIDIOC_DQBUF, &buf))
		{
            switch (errno)
            {
	            case EAGAIN:
	                    return BitmapPtr();
	
	            case EIO:
						AVG_TRACE(Logger::ERROR, "EIO");
						exit(-1);
						break;

	            case EINVAL:
						AVG_TRACE(Logger::ERROR, "EINVAL");
						exit(-1);
						break;

	            default:
	                    AVG_TRACE(Logger::ERROR, "VIDIOC_DQBUF");
	                    exit(-1);
            }
	    }
	    
	    pSrc = (unsigned char*)buffers_[buf.index].start;
	    
//	    AVG_TRACE(Logger::APP,"mmap() dequeueing buffer index=" << buf.index);
		break;
	}

	IntPoint size = getImgSize();

	// target bitmap and uchar* to
	BitmapPtr pCurBitmap;
	if (m_bColor) pCurBitmap = BitmapPtr(new Bitmap(size, B8G8R8X8));
	else pCurBitmap = BitmapPtr(new Bitmap(size, I8));
	
    unsigned char *pDest = pCurBitmap->getPixels();

 
 	ScopeTimer Timer(CameraConvertProfilingZone);

	switch (m_CamPF)
	{
	// fast BGR24 -> B8G8R8X8 conversion
	case V4L2_PIX_FMT_BGR24:
		for (int imgp = 0 ; imgp < size.x * size.y ; ++imgp)
		{
			*pDest++ = *pSrc++; // Blue
			*pDest++ = *pSrc++; // Green
			*pDest++ = *pSrc++; // Red
			*pDest++ = 0xFF;	// Alpha
		}
		break;
	
	case V4L2_PIX_FMT_GREY:
	{
		Bitmap TempBmp(size, I8, pSrc, size.x, false, "TempCameraBmp");
		pCurBitmap->copyPixels(TempBmp);
	}
		break;
		
	case V4L2_PIX_FMT_UYVY:
	{
		Bitmap TempBmp(size, YCbCr422, pSrc, size.x*2, false, "TempCameraBmp");
		pCurBitmap->copyPixels(TempBmp);
	}
		break;

	case V4L2_PIX_FMT_YUV420:
	{
		Bitmap TempBmp(size, YCbCr420p, pSrc, size.x, false, "TempCameraBmp");
		pCurBitmap->copyPixels(TempBmp);
	}
		break;
	
	/*
	// NOT SUPPORTED YET
	case V4L2_PIX_FMT_Y41P:
	{
	    Bitmap TempBmp(size, YCbCr411, pSrc, (int)(size.x*1.5), false, "TempCameraBmp");
	    pCurBitmap->copyPixels(TempBmp);
	}
	break; */
	}

	// enqueues free buffer for mmap
	if (ioMethod_ == IO_METHOD_MMAP)
	{
		if (-1 == xioctl (fd_, VIDIOC_QBUF, &buf))
		{
            AVG_TRACE(Logger::ERROR, "VIDIOC_DQBUF");
            exit(-1);
		}
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

double V4LCamera::getFrameRate() const
{
    return 25;
}

const std::string& V4LCamera::getMode() const
{
    return m_sMode;
}

int V4LCamera::featureToCID(const std::string& sFeature) const
{
	int v4lfeature;
	if (sFeature == "brightness") v4lfeature = V4L2_CID_BRIGHTNESS;
	else if (sFeature == "contrast") v4lfeature = V4L2_CID_CONTRAST;
	else if (sFeature == "gain") v4lfeature = V4L2_CID_GAIN;
	else if (sFeature == "exposure") v4lfeature = V4L2_CID_EXPOSURE;
	else if (sFeature == "whiteness") v4lfeature = V4L2_CID_WHITENESS;
	else if (sFeature == "gamma") v4lfeature = V4L2_CID_GAMMA;
	else if (sFeature == "saturation") v4lfeature = V4L2_CID_SATURATION;
	else
	{
		AVG_TRACE(Logger::APP, "Unsupported feature " << sFeature);
		return -1;
	}
	
	return v4lfeature;
}

bool V4LCamera::isCIDSupported(int v4lfeature) const
{
	struct v4l2_queryctrl queryctrl;
	
	CLEAR(queryctrl);
	queryctrl.id = v4lfeature;
	
	if (ioctl (fd_, VIDIOC_QUERYCTRL, &queryctrl) == -1)
	{
	        if (errno != EINVAL)
	        {
	                AVG_TRACE(Logger::ERROR,"VIDIOC_QUERYCTRL");
	                exit (-1);
	        } 
	        else return false;
	}
	else if (queryctrl.flags & V4L2_CTRL_FLAG_DISABLED) return false;
	else return true;
}

unsigned int V4LCamera::getFeature(const std::string& sFeature) const
{
	if (!m_bCameraAvailable)
	{
		AVG_TRACE(Logger::WARNING,"setFeature() called before opening device: ignored");
		return 0;
	}

	int v4lfeature = featureToCID(sFeature);
	
	isCIDSupported(v4lfeature);
	
	if (v4lfeature == -1 || isCIDSupported(v4lfeature) == false)
	{
		AVG_TRACE(Logger::ERROR,"Feature " << sFeature << " is not supported");
		return 0;
	}
	
	struct v4l2_control control;

	CLEAR(control);
	control.id = v4lfeature;

	if (ioctl (fd_, VIDIOC_G_CTRL, &control) == 0) return (unsigned int)control.value;
	else
	{
        AVG_TRACE(Logger::ERROR,"VIDIOC_G_CTRL");
        exit (-1);
	}
}

void V4LCamera::setFeature(const std::string& sFeature, int Value)
{
	if (!m_bCameraAvailable)
	{
		AVG_TRACE(Logger::WARNING,"setFeature() called before opening device: ignored");
		return;
	}

	int v4lfeature = featureToCID(sFeature);
	
	if (v4lfeature == -1 || !isCIDSupported(v4lfeature))
	{
		AVG_TRACE(Logger::ERROR,"Feature " << sFeature << " is not supported");
		return;
	}
	
	struct v4l2_control control;
	
    CLEAR(control);
    control.id = v4lfeature;
    control.value = Value;

    if (ioctl (fd_, VIDIOC_S_CTRL, &control) == -1)
    {
            AVG_TRACE(Logger::ERROR,"VIDIOC_S_CTRL");
            exit (-1);
    }
    else AVG_TRACE(Logger::APP, "Camera feature " << sFeature << " set to value " << Value);
}

void V4LCamera::startCapture()
{
//	AVG_TRACE(Logger::APP, "Entering startCapture()...");

	unsigned int i;
	enum v4l2_buf_type type;

	switch (ioMethod_)
	{
	case IO_METHOD_READ:
		/* Nothing to do. */
		break;

	case IO_METHOD_MMAP:
		for (i = 0; i < buffers_.size(); ++i)
		{
            struct v4l2_buffer buf;

    		CLEAR (buf);

    		buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    		buf.memory      = V4L2_MEMORY_MMAP;
    		buf.index       = i;

    		if (-1 == xioctl (fd_, VIDIOC_QBUF, &buf))
    		{
	        	AVG_TRACE(Logger::ERROR, "VIDIOC_QBUF");
	            exit (-1);
    		}
		}
		
		type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

		if (-1 == xioctl (fd_, VIDIOC_STREAMON, &type))
		{
        	AVG_TRACE(Logger::ERROR, "VIDIOC_STREAMON");
            exit (-1);
		}

		break;

	case IO_METHOD_USERPTR:
		for (i = 0; i < buffers_.size(); ++i)
		{
			struct v4l2_buffer buf;

			CLEAR (buf);

			buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
			buf.memory      = V4L2_MEMORY_USERPTR;
			buf.index       = i;
			buf.m.userptr	= (unsigned long) buffers_[i].start;
			buf.length      = buffers_[i].length;

			if (-1 == xioctl (fd_, VIDIOC_QBUF, &buf))
			{
	        	AVG_TRACE(Logger::ERROR, "VIDIOC_QBUF");
	            exit (-1);
			}
		}

		type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

		if (-1 == xioctl (fd_, VIDIOC_STREAMON, &type))
		{
        	AVG_TRACE(Logger::ERROR, "VIDIOC_STREAMON");
            exit (-1);
		}

		break;
	}

	AVG_TRACE(Logger::APP, "Capture started");

}

void V4LCamera::initDevice()
{
//	AVG_TRACE(Logger::APP, "Entering initDevice()...");
	
    struct v4l2_capability cap;
    struct v4l2_cropcap cropcap;
    struct v4l2_crop crop;
    struct v4l2_format fmt;
	unsigned int min;

    if (-1 == xioctl (fd_, VIDIOC_QUERYCAP, &cap))
    {
        if (EINVAL == errno)
        {
        	AVG_TRACE(Logger::ERROR, m_sDevice << " is not a valid V4L2 device");
            exit (-1);
        }
        else
        {
        	AVG_TRACE(Logger::ERROR, "VIDIOC_QUERYCAP error");
            exit(-1);
        }
    }

    if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE))
    {
        AVG_TRACE(Logger::ERROR, m_sDevice << " does not support capturing");
        exit(-1);
    }

	switch (ioMethod_)
	{
	case IO_METHOD_READ:
		if (!(cap.capabilities & V4L2_CAP_READWRITE))
		{
			AVG_TRACE(Logger::ERROR, m_sDevice << " does not support read i/o");
			exit (-1);
		}

		break;

	case IO_METHOD_MMAP:
	case IO_METHOD_USERPTR:
		if (!(cap.capabilities & V4L2_CAP_STREAMING))
		{
			AVG_TRACE(Logger::ERROR, m_sDevice << " does not support streaming i/os");
			exit (-1);
		}

		break;
	}
	

    /* Select video input, video standard and tune here. */
	CLEAR (cropcap);
    cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    if (xioctl(fd_, VIDIOC_CROPCAP, &cropcap) == 0)
    {
        crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        crop.c = cropcap.defrect; /* reset to default */

        if (-1 == xioctl (fd_, VIDIOC_S_CROP, &crop))
        {
            switch (errno)
            {
            case EINVAL:
                /* Cropping not supported. */
                break;
            default:
                /* Errors ignored. */
   		        break;
            }
        }
	}
	else
	{	
        	/* Errors ignored. */
	}

    CLEAR (fmt);

    fmt.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width       = getImgSize().x; 
    fmt.fmt.pix.height      = getImgSize().y;
    fmt.fmt.pix.pixelformat = m_CamPF;
    // TODO: deinterlace pipeline or one-field method
    fmt.fmt.pix.field       = V4L2_FIELD_INTERLACED;

    if (xioctl(fd_, VIDIOC_S_FMT, &fmt) == -1)
    {
		AVG_TRACE(Logger::ERROR, m_sDevice << " could not set image format");
		exit (-1);
	}

	AVG_TRACE(Logger::APP, "Format set to " << fmt.fmt.pix.width << "x" << fmt.fmt.pix.height << " interlaced");

	/* Note VIDIOC_S_FMT may change width and height. */

	/* Buggy driver paranoia. */
	min = fmt.fmt.pix.width * 2;
	if (fmt.fmt.pix.bytesperline < min)
		fmt.fmt.pix.bytesperline = min;
	min = fmt.fmt.pix.bytesperline * fmt.fmt.pix.height;
	if (fmt.fmt.pix.sizeimage < min)
		fmt.fmt.pix.sizeimage = min;

	switch (ioMethod_) {
	case IO_METHOD_READ:
		init_read (fmt.fmt.pix.sizeimage);
		break;

	case IO_METHOD_MMAP:
		init_mmap ();
		break;

	case IO_METHOD_USERPTR:
		init_userp (fmt.fmt.pix.sizeimage);
		break;
	}
	
	// TODO: string channel instead of numeric
	// select channel
	AVG_TRACE(Logger::APP, "Setting channel " << Channel_);
	if (xioctl(fd_, VIDIOC_S_INPUT, &Channel_) == -1)
	{
		AVG_TRACE(Logger::ERROR, "Cannot set MUX channel " << Channel_);
		exit(-1);
	}
	
	AVG_TRACE(Logger::APP, "V4L2 device initialized successfully");
	
	m_bCameraAvailable = true;
}

void V4LCamera::init_read (unsigned int buffer_size)
{
//	AVG_TRACE(Logger::APP, "entering init_read()...");
    buffers_.clear();
    Buffer tmp;

	tmp.length = buffer_size;
	tmp.start = malloc(buffer_size);

	if (!tmp.start) {
		fprintf (stderr, "Out of memory\n");
    	exit (-1);
	}
	
	buffers_.push_back(tmp);
//	AVG_TRACE(Logger::APP, "init_read() completed");
}

void V4LCamera::init_mmap()
{
	struct v4l2_requestbuffers req;
    CLEAR (req);

    req.count = 4;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;

	if (xioctl (fd_, VIDIOC_REQBUFS, &req) == -1)
	{
        if (EINVAL == errno)
        {
			AVG_TRACE(Logger::APP, m_sDevice << " does not support memory mapping");
			exit(-1);
        } else {
            //errno_exit ("VIDIOC_REQBUFS");
        }
    }
    
    AVG_TRACE(Logger::APP, "init_mmap(): " << req.count << " buffers requested");

    if (req.count < 2)
    {
		AVG_TRACE(Logger::APP, "Insufficient buffer memory on " << m_sDevice);
		exit(-1);
    }

    buffers_.clear();

    for (int i=0; i < req.count; ++i)
    {
		Buffer tmp;
		struct v4l2_buffer buf;

		CLEAR (buf);

		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;
		buf.index = i;

		if (xioctl (fd_, VIDIOC_QUERYBUF, &buf) == -1)
		{
			AVG_TRACE(Logger::ERROR, "VIDIOC_QUERYBUF index=" << i);
			exit (-1);
		}

		tmp.length = buf.length;
		
		tmp.start = mmap (NULL /* start anywhere */,
			buf.length,
			PROT_READ | PROT_WRITE /* required */,
			MAP_SHARED /* recommended */,
			fd_, buf.m.offset);

        if (MAP_FAILED == tmp.start)
		{
			AVG_TRACE(Logger::ERROR, "mmap() failed on buffer index=" << i);
			exit (-1);
		}
	 	   		
		buffers_.push_back(tmp);
    }
}

void V4LCamera::init_userp(unsigned int buffer_size)
{
	struct v4l2_requestbuffers req;
	CLEAR (req);

    req.count = 4;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_USERPTR;

    if (xioctl(fd_, VIDIOC_REQBUFS, &req) == -1)
    {
    	if (EINVAL == errno)
    	{
			AVG_TRACE(Logger::ERROR, m_sDevice << "does not support user pointer i/o");
			exit (-1);
        }
        else
        {
            //errno_exit ("VIDIOC_REQBUFS");
        }
    }

    buffers_.clear();

    for (int i=0; i<4; ++i)
    {
    	Buffer tmp;
        tmp.length = buffer_size;
        tmp.start = malloc (buffer_size);

        if (!tmp.start)
        {
			fprintf (stderr, "Out of memory\n");
    		//exit (EXIT_FAILURE);
		}
		
		buffers_.push_back(tmp);
    }
}


