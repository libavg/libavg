//
// $Id$
//

#include "VBlank.h"

#include "OGLHelper.h"

#include "../base/Logger.h"

#define XMD_H 1
#include "GL/gl.h"
#include "GL/glu.h"
#ifdef __APPLE__
#include <AGL/agl.h>
#endif

#include <sys/ioctl.h>
#include <sys/fcntl.h>

#include <unistd.h>
#include <cerrno>

using namespace std;

namespace avg {

#define DRM_VBLANK_RELATIVE 0x1;

struct drm_wait_vblank_request {
    int type;
    unsigned int sequence;
    unsigned long signal;
};

struct drm_wait_vblank_reply {
    int type;
    unsigned int sequence;
    long tval_sec;
    long tval_usec;
};

typedef union drm_wait_vblank {
    struct drm_wait_vblank_request request;
    struct drm_wait_vblank_reply reply;
} drm_wait_vblank_t;

#define DRM_IOCTL_BASE                  'd'
#define DRM_IOWR(nr,type)               _IOWR(DRM_IOCTL_BASE,nr,type)

#define DRM_IOCTL_WAIT_VBLANK           DRM_IOWR(0x3a, drm_wait_vblank_t)

static int drmWaitVBlank(int fd, drm_wait_vblank_t *vbl)
{
    int ret;
    int rc;

    do {
       ret = ioctl(fd, DRM_IOCTL_WAIT_VBLANK, vbl);
       vbl->request.type &= ~DRM_VBLANK_RELATIVE;
       rc = errno;
    } while (ret && rc == EINTR);

    return rc;
}

VBlank::VBlank()
{
}

VBlank::~VBlank()
{
    if (m_Method == VB_DRI) {
        close(m_dri_fd);
    }    
}

void VBlank::init()
{
#ifdef __APPLE__
    GLint swapInt = 1;
    // TODO: Find out why aglGetCurrentContext doesn't work.
    AGLContext Context = aglGetCurrentContext();
    if (Context == 0) {
        AVG_TRACE(Logger::WARNING,
                "Mac VBlank setup failed in aglGetCurrentContext(). Error was "
                << aglGetError() << ".");
    }
    bool bOk = aglSetInteger(Context, AGL_SWAP_INTERVAL, &swapInt);
    m_Method = VB_APPLE;
    
    if (bOk == GL_FALSE) {
        AVG_TRACE(Logger::WARNING,
                "Mac VBlank setup failed with error code " << aglGetError() << ".");
        m_Method = VB_KAPUTT;
    }
#else
    string sVendor = (const char *)(glGetString(GL_VENDOR));
    if (sVendor.find("NVIDIA") != string::npos && getenv("__GL_SYNC_TO_VBLANK") != 0) {
        m_Method = VB_NVIDIA;
    } else {
        if (sVendor.find("NVIDIA") != string::npos && 
            getenv("__GL_SYNC_TO_VBLANK") == 0) 
        {
            AVG_TRACE(Logger::WARNING, 
                    "Using NVIDIA graphics card but __GL_SYNC_TO_VBLANK not set.");
        }
    
        m_dri_fd = open("/dev/dri/card0", O_RDWR);
        if (m_dri_fd < 0)
        {
            AVG_TRACE(Logger::WARNING, "Could not open /dev/dri/card0 for vblank. Reason: "
                    <<strerror(errno));
            m_Method = VB_KAPUTT;
        } else {
            m_Method = VB_DRI;
        }
    }
#endif
    switch(m_Method) {
        case VB_DRI:
            AVG_TRACE(Logger::CONFIG, "Using DRI interface for vertical blank support.");
            break;
        case VB_NVIDIA:
            AVG_TRACE(Logger::CONFIG, "Using NVIDIA driver vertical blank support.");
            break;
        case VB_APPLE:
            AVG_TRACE(Logger::CONFIG, "Using Apple GL vertical blank support.");
            break;
        case VB_KAPUTT:
            AVG_TRACE(Logger::WARNING, "Vertical blank support disabled.");
            break;
    }
}

void VBlank::wait()
{
    switch (m_Method) {
        case VB_DRI:
            {
                drm_wait_vblank_t blank;
                blank.request.type = DRM_VBLANK_RELATIVE;
                blank.request.sequence = 1;
                int rc = drmWaitVBlank (m_dri_fd, &blank);
                if (rc) {
                    AVG_TRACE(Logger::WARNING, 
                            "Could not wait for vblank. Reason: "
                            << strerror(errno));
                    AVG_TRACE(Logger::WARNING, "Vertical blank support disabled.");
                    m_Method = VB_KAPUTT;
                }
            }
            break;
        default:
            break;
    }
}

}
