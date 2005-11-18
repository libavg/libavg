//
// $Id$
//

#include "VBlank.h"

#include "OGLHelper.h"

#include "../base/Logger.h"
#include "../base/Exception.h"

#define XMD_H 1
#include "GL/gl.h"
#include "GL/glu.h"
#include "GL/glx.h"
#ifdef __APPLE__
#include <AGL/agl.h>
#endif

#include <sys/ioctl.h>
#include <sys/fcntl.h>

#include <unistd.h>
#include <cerrno>

using namespace std;

namespace avg {

VBlank::VBlank()
    : m_Rate(0),
      m_Method(VB_KAPUTT),
      m_Mod(0),
      m_LastFrame(0),
      m_bFirstFrame(true)
{
}

VBlank::~VBlank()
{
}

bool VBlank::init(int Rate)
{
    if (Rate > 0) {
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
                    "Mac VBlank setup failed with error code " << 
                    aglGetError() << ".");
            m_Method = VB_KAPUTT;
        }
#else
        if (queryGLXExtension("GLX_SGI_video_sync")) {
            m_Method = VB_SGI;
            m_bFirstFrame = true;
        } else {
            m_Method = VB_KAPUTT;
        }
    } else {
        m_Method = VB_KAPUTT;
    }
#endif
    m_Rate = Rate;
    switch(m_Method) {
        case VB_SGI:
            AVG_TRACE(Logger::CONFIG, 
                    "Using SGI interface for vertical blank support.");
            break;
        case VB_APPLE:
            AVG_TRACE(Logger::CONFIG, "Using Apple GL vertical blank support.");
            break;
        case VB_KAPUTT:
            m_Rate = Rate;
            AVG_TRACE(Logger::CONFIG, "Vertical blank support disabled.");
            break;
    }
    return (m_Method != VB_KAPUTT);
}

void VBlank::wait()
{
    if (m_Method == VB_SGI) {
        unsigned int count;
        int err = glXWaitVideoSyncSGI(m_Rate, m_Mod, &count);
        OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "VBlank::glXWaitVideoSyncSGI");
        if (err) {
            AVG_TRACE(Logger::ERROR, "glXWaitVideoSyncSGI returned " << err << ".");
            AVG_TRACE(Logger::ERROR, "Rate was " << m_Rate << ", Mod was " << m_Mod);
            AVG_TRACE(Logger::ERROR, "Disabling VBlank support.");
            m_Method = VB_KAPUTT;
            m_Rate = 0;
            return;
        }
        m_Mod = count % m_Rate;
        if (!m_bFirstFrame && int(count) != m_LastFrame+m_Rate) {
            AVG_TRACE(Logger::PROFILE_LATEFRAMES, count-m_LastFrame
                    << " VBlank intervals missed, shound be " << m_Rate);
        }
        m_LastFrame = count;
        m_bFirstFrame = false;
    }
}

bool VBlank::isActive() const
{
    return (m_Method != VB_KAPUTT);
}

}
