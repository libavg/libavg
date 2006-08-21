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

#include "DisplayEngine.h"

#include "../base/Logger.h"
#include "../base/ScopeTimer.h"
#include "../base/Profiler.h"
#include "../base/Exception.h"

using namespace std;

namespace avg {

DisplayEngine::DisplayEngine()
    : m_NumFrames(0),
      m_VBRate(0),
      m_Framerate(30),
      m_bInitialized(false)
{
}

DisplayEngine::~DisplayEngine()
{
}


void DisplayEngine::initRender()
{
    bool bUseVBlank = false;
    if (m_VBRate != 0) {
        bUseVBlank = initVBlank(m_VBRate);
        m_Framerate = getRefreshRate()/m_VBRate;
        if (!bUseVBlank) {
            AVG_TRACE(Logger::WARNING, "Using framerate of " << m_Framerate << 
                    " instead of VBRate of " << m_VBRate);
        }
    }
    m_NumFrames = 0;
    m_FramesTooLate = 0;
    m_TimeSpentWaiting = 0;
    m_StartTime = TimeSource::get()->getCurrentMillisecs();
    m_LastFrameTime = m_StartTime;
    m_bInitialized = true;
    if (!bUseVBlank) {
        m_VBRate = 0;
    }
}

void DisplayEngine::deinitRender()
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
    if (m_Framerate != 0) {
        AVG_TRACE(Logger::PROFILE, "  Framerate goal was: " << m_Framerate);
    }
    m_bInitialized = false;
}

void DisplayEngine::setFramerate(double rate)
{
    if (rate != 0 && m_bInitialized) {
        // TODO: Is this nessesary?
        initVBlank(0);
    }
    m_Framerate = rate;
    m_VBRate = 0;
}

double DisplayEngine::getFramerate()
{
    return m_Framerate;
}

bool DisplayEngine::setVBlankRate(int rate) {
    m_VBRate = rate;
    if (m_bInitialized) {
        bool bOK = initVBlank(rate);
        if (bOK && rate != 0) { 
            m_Framerate = 0;
            return true;
        } else {
            return false;
        }
    } else {
        return true;
    }
}

bool DisplayEngine::wasFrameLate()
{
    return m_bFrameLate;
}

static ProfilingZone WaitProfilingZone("  Render - wait");

void DisplayEngine::frameWait()
{
    ScopeTimer Timer(WaitProfilingZone);

    m_NumFrames++;
    m_FrameWaitStartTime = TimeSource::get()->getCurrentMillisecs();
    m_TargetTime = m_LastFrameTime+(long long)(1000/m_Framerate);
    if (m_VBRate != 0) {
        m_bFrameLate = !vbWait(m_VBRate);
    } else {
        m_bFrameLate = false;
        if (m_FrameWaitStartTime <= m_TargetTime) {
            long long WaitTime = m_TargetTime-m_FrameWaitStartTime;
            if (WaitTime > 200) {
                AVG_TRACE (Logger::WARNING, 
                        "DisplayEngine: waiting " << WaitTime << " ms.");
            }
            TimeSource::get()->sleepUntil(m_TargetTime);
        }
    }
}

void DisplayEngine::checkJitter()
{
    m_LastFrameTime = TimeSource::get()->getCurrentMillisecs();
    int maxDelay;
    if (m_VBRate == 0) {
        maxDelay = 2;
    } else {
        maxDelay = 6;
    }
    if (m_LastFrameTime - m_TargetTime > maxDelay || m_bFrameLate) {
        AVG_TRACE (Logger::PROFILE_LATEFRAMES, 
                "DisplayEngine: frame too late by " 
                << m_LastFrameTime - m_TargetTime << " ms.");
        m_bFrameLate = true;
        m_FramesTooLate++;
    }
    m_TimeSpentWaiting += m_LastFrameTime-m_FrameWaitStartTime;
}
    
}

