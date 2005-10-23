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
        double TotalTime = double(TimeSource::get()->getCurrentTicks()-m_StartTime)/1000;
        AVG_TRACE(Logger::PROFILE, "  Total time: " << TotalTime << " seconds");
        AVG_TRACE(Logger::PROFILE, "  Framerate achieved: " 
             << (m_NumFrames+1)/TotalTime);
        AVG_TRACE(Logger::PROFILE, "  Percent of time spent waiting: " 
             << double (m_TimeSpentWaiting)/(10*TotalTime));
//        AVG_TRACE(Logger::PROFILE, "  Avg. frame jitter: " 
//             << double(m_TotalJitter)/(m_NumFrames*1000));
        AVG_TRACE(Logger::PROFILE, "  Frames too late: " << m_FramesTooLate);
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

void FramerateManager::FrameWait(bool bVBlank)
{
    ScopeTimer Timer(WaitProfilingZone);
    
    m_NumFrames++;
    m_NumRegularFrames++;

    m_FrameWaitStartTime = TimeSource::get()->getCurrentTicks();
    long long TargetTime = m_LastFrameTime+(long long)((1000/m_Rate)*m_NumRegularFrames);
    if (m_FrameWaitStartTime <= TargetTime) 
    {
        long long WaitTime = TargetTime-m_FrameWaitStartTime;
        if (bVBlank) {
            // Don't wait quite as long so we don't miss the vblank interval.
            if (WaitTime >= 5) {
                WaitTime -= 5;
            }
        }
        if (WaitTime > 200) {
            AVG_TRACE (Logger::PROFILE, 
                    "FramerateManager: waiting " << TargetTime-m_FrameWaitStartTime 
                    << " ms.");
//            AVG_TRACE (Logger::PROFILE,
//                    "  TargetTime: " << TargetTime << ", m_FrameWaitStartTime: " 
//                    << m_FrameWaitStartTime);
        }
        m_bLastFrameLate = false;
        TimeSource::get()->sleepUntil(TargetTime);
    }
    else
    {
        if (m_FrameWaitStartTime - TargetTime > 2) {
            m_FramesTooLate++;
        }
        m_NumRegularFrames = 0;
        m_bLastFrameLate = true;
//        AVG_TRACE (Logger::PROFILE, 
//                "FramerateManager: frame too late by " 
//                << m_FrameWaitStartTime - TargetTime << " ms.");
    }
}

void FramerateManager::CheckJitter() {
    long long CurTime = TimeSource::get()->getCurrentTicks();
    long long TargetTime = m_LastFrameTime+(int)((1000/m_Rate)*m_NumRegularFrames);
//    if (CurTime >= TargetTime+2 && !m_bLastFrameLate) {
//        AVG_TRACE(Logger::PROFILE, "FramerateManager: swap too late by " <<
//                CurTime-TargetTime << " ms."); 
//    }
//    m_TotalJitter += m_FrameWaitStartTime - TargetTime;
    if (m_bLastFrameLate) {
        m_LastFrameTime = TimeSource::get()->getCurrentTicks();
    }
    m_TimeSpentWaiting += TimeSource::get()->getCurrentTicks()-m_FrameWaitStartTime;
}

}
