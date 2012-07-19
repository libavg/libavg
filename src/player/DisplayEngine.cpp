//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2011 Ulrich von Zadow
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

#include "DisplayEngine.h"

#include "../base/Logger.h"
#include "../base/ScopeTimer.h"
#include "../base/TimeSource.h"
#include "../base/Exception.h"

#include "../graphics/GLContext.h"

using namespace std;

namespace avg {

DisplayEngine::DisplayEngine()
    : m_NumFrames(0),
      m_VBRate(0),
      m_Framerate(60),
      m_bInitialized(false),
      m_EffFramerate(0)
{
}

DisplayEngine::~DisplayEngine()
{
}


void DisplayEngine::initRender()
{
    m_NumFrames = 0;
    m_FramesTooLate = 0;
    m_TimeSpentWaiting = 0;
    m_StartTime = TimeSource::get()->getCurrentMicrosecs();
    m_LastFrameTime = m_StartTime;
    m_bInitialized = true;
    if (m_VBRate != 0) {
        setVBlankRate(m_VBRate);
    } else {
        setFramerate(m_Framerate);
    }
}

void DisplayEngine::deinitRender()
{
    AVG_TRACE(Logger::PROFILE, "Framerate statistics: ");
    AVG_TRACE(Logger::PROFILE, "  Total frames: " << m_NumFrames);
    float TotalTime = float(TimeSource::get()->getCurrentMicrosecs()
            -m_StartTime)/1000000;
    AVG_TRACE(Logger::PROFILE, "  Total time: " << TotalTime << " seconds");
    float actualFramerate = (m_NumFrames+1)/TotalTime;
    AVG_TRACE(Logger::PROFILE, "  Framerate achieved: " 
            << actualFramerate);
    AVG_TRACE(Logger::PROFILE, "  Frames too late: " << m_FramesTooLate);
    AVG_TRACE(Logger::PROFILE, "  Percent of time spent waiting: " 
            << float (m_TimeSpentWaiting)/(10000*TotalTime));
    if (m_Framerate != 0) {
        AVG_TRACE(Logger::PROFILE, "  Framerate goal was: " << m_Framerate);
        if (m_Framerate*2 < actualFramerate && m_NumFrames > 10) {
            AVG_TRACE(Logger::WARNING, 
                    "Actual framerate was a lot higher than framerate goal. Is vblank sync forced off?");
        }
    }
    m_bInitialized = false;
}

void DisplayEngine::setFramerate(float rate)
{
    if (rate != 0 && m_bInitialized) {
        GLContext::getMain()->initVBlank(0);
    }
    m_Framerate = rate;
    m_VBRate = 0;
}

float DisplayEngine::getFramerate()
{
    return m_Framerate;
}

float DisplayEngine::getEffectiveFramerate()
{
    return m_EffFramerate;
}

void DisplayEngine::setVBlankRate(int rate)
{
    m_VBRate = rate;
    if (m_bInitialized) {
        bool bOK = GLContext::getMain()->initVBlank(rate);
        m_Framerate = getRefreshRate()/m_VBRate;
        if (!bOK || rate == 0) { 
            AVG_TRACE(Logger::WARNING, "Using framerate of " << m_Framerate << 
                    " instead of VBRate of " << m_VBRate);
            m_VBRate = 0;
        }
    }
}

bool DisplayEngine::wasFrameLate()
{
    return m_bFrameLate;
}

static ProfilingZoneID WaitProfilingZone("Render - wait");

void DisplayEngine::frameWait()
{
    ScopeTimer Timer(WaitProfilingZone);

    m_NumFrames++;

    m_FrameWaitStartTime = TimeSource::get()->getCurrentMicrosecs();
    m_TargetTime = m_LastFrameTime+(long long)(1000000/m_Framerate);
    m_bFrameLate = false;
    if (m_VBRate == 0) {
        if (m_FrameWaitStartTime <= m_TargetTime) {
            long long WaitTime = (m_TargetTime-m_FrameWaitStartTime)/1000;
            if (WaitTime > 5000) {
                AVG_TRACE (Logger::WARNING, 
                        "DisplayEngine: waiting " << WaitTime << " ms.");
            }
            TimeSource::get()->sleepUntil(m_TargetTime/1000);
        }
    }
}

long long DisplayEngine::getDisplayTime() 
{
    return (m_LastFrameTime-m_StartTime)/1000;
}

void DisplayEngine::checkJitter()
{
    if (m_LastFrameTime == 0) {
        m_EffFramerate = 0;
    } else {
        long long CurIntervalTime = TimeSource::get()->getCurrentMicrosecs()
                -m_LastFrameTime;
        m_EffFramerate = 1000000.0f/CurIntervalTime;
    }

    long long frameTime = TimeSource::get()->getCurrentMicrosecs();
    int maxDelay;
    if (m_VBRate == 0) {
        maxDelay = 2;
    } else {
        maxDelay = 6;
    }
    if ((frameTime - m_TargetTime)/1000 > maxDelay || m_bFrameLate) {
        m_bFrameLate = true;
        m_FramesTooLate++;
    }

    m_LastFrameTime = frameTime;
    m_TimeSpentWaiting += m_LastFrameTime-m_FrameWaitStartTime;
//    cerr << m_LastFrameTime << ", m_FrameWaitStartTime=" << m_FrameWaitStartTime << endl;
//    cerr << m_TimeSpentWaiting << endl;
}
   
}
