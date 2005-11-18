//
// $Id$
//

#include "FramerateManager.h"
#include "Player.h"
#include "OGLHelper.h"

#include "../base/TimeSource.h"
#include "../base/Logger.h"
#include "../base/ScopeTimer.h"
#include "../base/Profiler.h"
#include "../base/Exception.h"

#include "GL/glx.h"

#include <sys/time.h>
#include <unistd.h>
#include <X11/Xlib.h>
#include <X11/extensions/xf86vmode.h>

#include <iostream>
#include <sstream>

using namespace std;

namespace avg {

double FramerateManager::s_RefreshRate = 0.0;

FramerateManager::FramerateManager ()
    : m_NumFrames(0),
      m_Rate(25),
      m_bInitialized(false),
      m_VBlankRate(1),
      m_VBMod(0)
{
    calcRefreshRate();
}

FramerateManager::~FramerateManager ()
{
}

void FramerateManager::Init()
{
    m_NumFrames = 0;
    m_FramesTooLate = 0;
    m_TimeSpentWaiting = 0;
    m_StartTime = TimeSource::get()->getCurrentMillisecs();
    m_LastFrameTime = m_StartTime;
    m_bInitialized = true;
    initVBlank();
    if (m_VBMethod == VB_NONE) {
        m_VBlankRate = 0;
    }
}

void FramerateManager::Deinit()
{
    AVG_TRACE(Logger::PROFILE, "Framerate statistics: ");
    AVG_TRACE(Logger::PROFILE, "  Total frames: " << m_NumFrames);
    double TotalTime = double(TimeSource::get()->getCurrentMillisecs()
            -m_StartTime)/1000;
    AVG_TRACE(Logger::PROFILE, "  Total time: " << TotalTime << " seconds");
    AVG_TRACE(Logger::PROFILE, "  Framerate achieved: " 
            << (m_NumFrames+1)/TotalTime);
    AVG_TRACE(Logger::PROFILE, "  Frames too late: " << m_FramesTooLate);
    AVG_TRACE(Logger::PROFILE, "  Percent of time spent waiting: " 
            << double (m_TimeSpentWaiting)/(10*TotalTime));
    if (m_VBMethod == VB_NONE) {
        AVG_TRACE(Logger::PROFILE, "  Framerate goal was: " << m_Rate);
    }
    m_bInitialized = false;
}

void FramerateManager::SetRate(double Rate)
{
    m_Rate = Rate;
    m_VBlankRate = 0;
    if (m_bInitialized) {
        initVBlank();
    }
}

bool FramerateManager::SetVBlankRate(int Rate)
{
    m_VBlankRate = Rate;
    if (m_bInitialized) {
        initVBlank();
        return (m_VBMethod != VB_NONE);
    } else {
        return true;
    }
}

double FramerateManager::GetRate()
{
    return m_Rate;
}

double FramerateManager::GetRefreshRate()
{
    if (s_RefreshRate == 0.0) {
        calcRefreshRate();
    }
    return s_RefreshRate;
}

static ProfilingZone WaitProfilingZone("  Render - wait");

void FramerateManager::FrameWait()
{
    ScopeTimer Timer(WaitProfilingZone);
    
    m_NumFrames++;
    m_FrameWaitStartTime = TimeSource::get()->getCurrentMillisecs();
    switch(m_VBMethod) {
        case VB_SGI: {
                unsigned int count;
                int err = glXWaitVideoSyncSGI(m_VBlankRate, m_VBMod, &count);
                OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
                        "VBlank::glXWaitVideoSyncSGI");
                if (err) {
                    AVG_TRACE(Logger::ERROR, "glXWaitVideoSyncSGI returned " 
                            << err << ".");
                    AVG_TRACE(Logger::ERROR, "Rate was " << m_VBlankRate 
                            << ", Mod was " << m_VBMod);
                    AVG_TRACE(Logger::ERROR, "Disabling VBlank support.");
                    m_VBMethod = VB_NONE;
                    m_VBlankRate = 0;
                    return;
                }
                m_VBMod = count % m_VBlankRate;
                if (!m_bFirstVBFrame && int(count) != m_LastVBCount+m_VBlankRate) {
                    AVG_TRACE(Logger::PROFILE_LATEFRAMES, count-m_LastVBCount
                            << " VBlank intervals missed, shound be " 
                            << m_VBlankRate);
                    m_FramesTooLate++;
                }
                m_LastVBCount = count;
                m_bFirstVBFrame = false;
            }
            break;
        case VB_APPLE:
            // Nothing needs to be done.
            break;
        case VB_NONE:
            m_TargetTime = m_LastFrameTime+(long long)(1000/m_Rate);
            if (m_FrameWaitStartTime <= m_TargetTime) {
                long long WaitTime = m_TargetTime-m_FrameWaitStartTime;
                if (WaitTime > 200) {
                    AVG_TRACE (Logger::WARNING, 
                            "FramerateManager: waiting " << WaitTime << " ms.");
                }
                TimeSource::get()->sleepUntil(m_TargetTime);
            }
            break;
    }
}

void FramerateManager::CheckJitter() {
    m_LastFrameTime = TimeSource::get()->getCurrentMillisecs();
    if (m_VBMethod == VB_NONE) {
        if (m_LastFrameTime - m_TargetTime > 2) {
            AVG_TRACE (Logger::PROFILE_LATEFRAMES, 
                    "FramerateManager: frame too late by " 
                    << m_LastFrameTime - m_TargetTime << " ms.");
            m_FramesTooLate++;
        }
    }
    m_TimeSpentWaiting += m_LastFrameTime-m_FrameWaitStartTime;
}

void FramerateManager::initVBlank()
{
    if (m_VBlankRate > 0) {
#ifdef __APPLE__
        GLint swapInt = m_VBlankRate;
        // TODO: Find out why aglGetCurrentContext doesn't work.
        AGLContext Context = aglGetCurrentContext();
        if (Context == 0) {
            AVG_TRACE(Logger::WARNING,
                    "Mac VBlank setup failed in aglGetCurrentContext(). Error was "
                    << aglGetError() << ".");
        }
        bool bOk = aglSetInteger(Context, AGL_SWAP_INTERVAL, &swapInt);
        m_VBMethod = VB_APPLE;

        if (bOk == GL_FALSE) {
            AVG_TRACE(Logger::WARNING,
                    "Mac VBlank setup failed with error code " << 
                    aglGetError() << ".");
            m_VBMethod = VB_NONE;
        }
#else
        if (queryGLXExtension("GLX_SGI_video_sync")) {
            m_VBMethod = VB_SGI;
            m_bFirstVBFrame = true;
            if (getenv("__GL_SYNC_TO_VBLANK") != 0) 
            {
                AVG_TRACE(Logger::WARNING, 
                        "__GL_SYNC_TO_VBLANK set. This interferes with libavg vblank handling.");
                m_VBMethod = VB_NONE;
            }
        } else {
            m_VBMethod = VB_NONE;
        }
    } else {
        m_VBMethod = VB_NONE;
    }
#endif
    switch(m_VBMethod) {
        case VB_SGI:
            AVG_TRACE(Logger::CONFIG, 
                    "Using SGI interface for vertical blank support.");
            break;
        case VB_APPLE:
            AVG_TRACE(Logger::CONFIG, "Using Apple GL vertical blank support.");
            break;
        case VB_NONE:
            AVG_TRACE(Logger::CONFIG, "Vertical blank support disabled.");
            break;
    }
}


void FramerateManager::calcRefreshRate() {
    double lastRefreshRate = s_RefreshRate;
#ifdef __APPLE__
#warning calcRefreshRate unimplemented on Mac!
    s_RefreshRate = 120;
#else 
    Display * display = XOpenDisplay(0);
    
    int PixelClock;
    XF86VidModeModeLine mode_line;
    bool bOK = XF86VidModeGetModeLine (display, DefaultScreen(display), 
            &PixelClock, &mode_line);
    if (!bOK) {
        AVG_TRACE (Logger::WARNING, 
                "Could not get current refresh rate (XF86VidModeGetModeLine failed).");
    }
    double HSyncRate = PixelClock*1000.0/mode_line.htotal;
    s_RefreshRate = HSyncRate/mode_line.vtotal;
#endif
    if (lastRefreshRate != s_RefreshRate) {
        AVG_TRACE(Logger::CONFIG, "Vertical Refresh Rate: " << s_RefreshRate);
    }

}

}
