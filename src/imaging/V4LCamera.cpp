
#include "V4LCamera.h"

#include "../base/Logger.h"

#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>
#include <linux/videodev2.h>

#define CLEAR(x) memset (&(x), 0, sizeof (x))

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

V4LCamera::V4LCamera(std::string sDevice, double FrameRate, std::string sMode, 
	bool bColor)
	: fd_(-1), m_sDevice(sDevice), ioMethod_(V4LCamera::IO_METHOD_MMAP)
    /*: 
      m_FrameRate(FrameRate),
      m_sMode(sMode),
      m_bColor(bColor),
      m_bCameraAvailable(false) */
{

}

V4LCamera::~V4LCamera() 
{}

void V4LCamera::open()
{
	struct stat st; 

    if ( stat(m_sDevice.c_str(), &st) == -1) {
    	AVG_TRACE(Logger::ERROR, "Unable to open v4l device " << m_sDevice);
        // TODO: Disable camera instead of exit(-1).
        exit(-1);
    }

    if (!S_ISCHR (st.st_mode)) {
    	AVG_TRACE(Logger::ERROR, m_sDevice + " is not a v4l device");
        // TODO: Disable camera instead of exit(-1).
        exit(-1);
    }

    fd_ = ::open(m_sDevice.c_str(), O_RDWR /* required */ | O_NONBLOCK, 0);

    if (fd_ == -1) {
    	AVG_TRACE(Logger::ERROR, "Unable to open v4l device " << m_sDevice);
        // TODO: Disable camera instead of exit(-1).
        exit(-1);
    }
}

void V4LCamera::close()
{
	if ( ::close(fd_) == -1)
        AVG_TRACE(Logger::ERROR, "Error on closing v4l device");

    fd_ = -1;
}

IntPoint V4LCamera::getImgSize()
{}

BitmapPtr V4LCamera::getImage(bool bWait)
{}

bool V4LCamera::isCameraAvailable()
{
    return false;
}

const std::string& V4LCamera::getDevice() const
{
    return m_sDevice;
}

double V4LCamera::getFrameRate() const
{
    return 0;
}

const std::string& V4LCamera::getMode() const
{
    return "";
}


unsigned int V4LCamera::getFeature(const std::string& sFeature) const
{
    return 0;
}

void V4LCamera::setFeature(const std::string& sFeature, int Value)
{}

void V4LCamera::initDevice()
{
    struct v4l2_capability cap;
    struct v4l2_cropcap cropcap;
    struct v4l2_crop crop;
    struct v4l2_format fmt;
	unsigned int min;

    if (-1 == xioctl (fd_, VIDIOC_QUERYCAP, &cap)) {
        if (EINVAL == errno) {
            fprintf (stderr, "%s is no V4L2 device\n", m_sDevice.c_str());
            //exit (EXIT_FAILURE);
        } else {
            //errno_exit ("VIDIOC_QUERYCAP");
        }
    }

    if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
        fprintf (stderr, "%s is no video capture device\n", m_sDevice.c_str());
        //exit (EXIT_FAILURE);
    }

	switch (ioMethod_) {
	case IO_METHOD_READ:
		if (!(cap.capabilities & V4L2_CAP_READWRITE)) {
			fprintf (stderr, "%s does not support read i/o\n", m_sDevice.c_str());
			//exit (EXIT_FAILURE);
		}

		break;

	case IO_METHOD_MMAP:
	case IO_METHOD_USERPTR:
		if (!(cap.capabilities & V4L2_CAP_STREAMING)) {
			fprintf (stderr, "%s does not support streaming i/o\n", m_sDevice.c_str());
			//exit (EXIT_FAILURE);
		}

		break;
	}


    /* Select video input, video standard and tune here. */
	CLEAR (cropcap);
    cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    if (xioctl(fd_, VIDIOC_CROPCAP, &cropcap) == 0) {
        crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        crop.c = cropcap.defrect; /* reset to default */

        if (-1 == xioctl (fd_, VIDIOC_S_CROP, &crop)) {
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

    CLEAR (fmt);

    fmt.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width       = 640; 
    fmt.fmt.pix.height      = 480;
	fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
    fmt.fmt.pix.field       = V4L2_FIELD_INTERLACED;

    if (xioctl(fd_, VIDIOC_S_FMT, &fmt) == -1)
        //errno_exit ("VIDIOC_S_FMT");

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
	
}

void V4LCamera::init_read (unsigned int buffer_size)
{
    buffers_.clear();
    Buffer tmp;

	tmp.length = buffer_size;
	tmp.start = malloc(buffer_size);

	if (!tmp.start) {
		fprintf (stderr, "Out of memory\n");
    	//exit (EXIT_FAILURE);
	}
	
	buffers_.push_back(tmp);
}

void V4LCamera::init_mmap()
{
	struct v4l2_requestbuffers req;
    CLEAR (req);

    req.count = 4;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;

	if (xioctl (fd_, VIDIOC_REQBUFS, &req) == -1) {
        if (EINVAL == errno) {
            fprintf (stderr, "%s does not support memory mapping\n", 
            	m_sDevice.c_str());
            //exit (EXIT_FAILURE);
        } else {
            //errno_exit ("VIDIOC_REQBUFS");
        }
    }

    if (req.count < 2) {
        fprintf (stderr, "Insufficient buffer memory on %s\n", 
        	m_sDevice.c_str());
        //exit (EXIT_FAILURE);
    }

    buffers_.clear();

    for (int i=0; i < req.count; ++i) {
    	Buffer tmp;
	    struct v4l2_buffer buf;
        CLEAR (buf);

	    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;

	    if (xioctl (fd_, VIDIOC_QUERYBUF, &buf) == -1)
            ;//errno_exit ("VIDIOC_QUERYBUF");

        tmp.length = buf.length;
        tmp.start =
        mmap (NULL /* start anywhere */,
          buf.length,
          PROT_READ | PROT_WRITE /* required */,
          MAP_SHARED /* recommended */,
          fd_, buf.m.offset);

        if (MAP_FAILED == tmp.start)
 	   		;//errno_exit ("mmap");
 	   		
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

    if (xioctl(fd_, VIDIOC_REQBUFS, &req) == -1) {
    	if (EINVAL == errno) {
        	fprintf (stderr, "%s does not support "
                                 "user pointer i/o\n", m_sDevice.c_str());
            //exit (EXIT_FAILURE);
        } else {
            //errno_exit ("VIDIOC_REQBUFS");
        }
    }

    buffers_.clear();

    for (int i=0; i<4; ++i) {
    	Buffer tmp;
        tmp.length = buffer_size;
        tmp.start = malloc (buffer_size);

        if (!tmp.start) {
			fprintf (stderr, "Out of memory\n");
    		//exit (EXIT_FAILURE);
		}
		
		buffers_.push_back(tmp);
    }
}
