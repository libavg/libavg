//
// $Id$
//

#include "FramerateManager.h"
#include "Player.h"

#include "../base/TimeSource.h"
#include "../base/Logger.h"
#include "../base/ScopeTimer.h"
#include "../base/Profiler.h"

#include <sys/time.h>
#include <unistd.h>

#include <iostream>
#include <sstream>

using namespace std;

namespace avg {

FramerateManager::FramerateManager ()
    : m_NumFrames(0),
      m_NumRegularFrames(0),
      m_Rate(0),
      m_bLastFrameLate(false),
      m_TotalJitter(0)
{
}

FramerateManager::~FramerateManager ()
{
    if (m_Rate != 0) {
        AVG_TRACE(Logger::PROFILE, "Framerate statistics: ");
        AVG_TRACE(Logger::PROFILE, "  Framerate goal was: " << m_Rate);
        AVG_TRACE(Logger::PROFILE, "  Total frames: " << m_NumFrames);
        AVG_TRACE(Logger::PROFILE, "  Frames too late: " << m_FramesTooLate);
        double TotalTime = double(TimeSource::get()->getCurrentTicks()-m_StartTime)/1000;
        AVG_TRACE(Logger::PROFILE, "  Total time: " << TotalTime << " seconds");
        AVG_TRACE(Logger::PROFILE, "  Framerate achieved: " 
             << (m_NumFrames+1)/TotalTime);
        AVG_TRACE(Logger::PROFILE, "  Time spent waiting: " 
             << double (m_TimeSpentWaiting)/1000 << " seconds" );
        AVG_TRACE(Logger::PROFILE, "  Percent of time spent waiting: " 
             << double (m_TimeSpentWaiting)/(10*TotalTime));
        AVG_TRACE(Logger::PROFILE, "  Avg. frame jitter: " 
             << double(m_TotalJitter)/(m_NumFrames*1000));
    }
}

void FramerateManager::SetRate(double Rate)
{
    m_Rate = Rate;
    m_NumFrames = 0;
    m_NumRegularFrames = 0;
    m_FramesTooLate = 0;
    m_TimeSpentWaiting = 0;
    m_LastFrameTime = TimeSource::get()->getCurrentTicks();
    m_StartTime = TimeSource::get()->getCurrentTicks();
    m_TotalJitter = 0;
}

double FramerateManager::GetRate()
{
    return m_Rate;
}

static ProfilingZone WaitProfilingZone("  Render - wait");

void FramerateManager::FrameWait()
{
    ScopeTimer Timer(WaitProfilingZone);
    
    m_NumFrames++;
    m_NumRegularFrames++;

    long long CurTime = TimeSource::get()->getCurrentTicks();
    long long TargetTime = m_LastFrameTime+(long long)((1000/m_Rate)*m_NumRegularFrames);
    if (CurTime <= TargetTime) 
    {
        long long WaitTime = TargetTime-CurTime;
        if (WaitTime > 200) {
            AVG_TRACE (Logger::PROFILE, 
                    "FramerateManager: waiting " << TargetTime-CurTime << " ms.");
//            AVG_TRACE (Logger::PROFILE,
//                    "  TargetTime: " << TargetTime << ", CurTime: " << CurTime);
        }
        m_bLastFrameLate = false;
        TimeSource::get()->sleepUntil(TargetTime);
    }
    else
    {
        m_FramesTooLate++;
        m_NumRegularFrames = 0;
        m_LastFrameTime = TimeSource::get()->getCurrentTicks();
        m_bLastFrameLate = true;
    }
    m_TimeSpentWaiting += TimeSource::get()->getCurrentTicks()-CurTime;
}

void FramerateManager::CheckJitter() {
    long long CurTime = TimeSource::get()->getCurrentTicks();
    long long TargetTime = m_LastFrameTime+(int)((1000/m_Rate)*m_NumRegularFrames);
    if (CurTime >= TargetTime+2 && !m_bLastFrameLate) {
//        AVG_TRACE(Logger::PROFILE, "FramerateManager: swap too late by " <<
//                CurTime-TargetTime << " ms."); 
    }
    m_TotalJitter += CurTime - TargetTime;
}

}
