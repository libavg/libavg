//
// $Id$
//

#include "AVGFramerateManager.h"
#include "AVGTimeSource.h"
#include "AVGPlayer.h"
#include "AVGLogger.h"

#include <sys/time.h>
#include <unistd.h>

#include <iostream>
#include <sstream>

using namespace std;

AVGFramerateManager::AVGFramerateManager ()
    : m_NumFrames(0),
      m_NumRegularFrames(0),
      m_Rate(0),
      m_bLastFrameLate(false),
      m_TotalJitter(0)
{
}

AVGFramerateManager::~AVGFramerateManager ()
{
    if (m_Rate != 0) {
        AVG_TRACE(AVGPlayer::DEBUG_PROFILE, "Framerate statistics: ");
        AVG_TRACE(AVGPlayer::DEBUG_PROFILE, "  Framerate goal was: " << m_Rate);
        AVG_TRACE(AVGPlayer::DEBUG_PROFILE, "  Total frames: " << m_NumFrames);
        AVG_TRACE(AVGPlayer::DEBUG_PROFILE, "  Frames too late: " << m_FramesTooLate);
        double TotalTime = double(AVGTimeSource::getCurrentTicks()-m_StartTime)/1000;
        AVG_TRACE(AVGPlayer::DEBUG_PROFILE, "  Total time: " << TotalTime << " seconds");
        AVG_TRACE(AVGPlayer::DEBUG_PROFILE, "  Framerate achieved: " 
             << (m_NumFrames+1)/TotalTime);
        AVG_TRACE(AVGPlayer::DEBUG_PROFILE, "  Time spent waiting: " << double (m_TimeSpentWaiting)/1000 
             << " seconds" );
        AVG_TRACE(AVGPlayer::DEBUG_PROFILE, "  Percent of time spent waiting: " 
             << double (m_TimeSpentWaiting)/(10*TotalTime));
        AVG_TRACE(AVGPlayer::DEBUG_PROFILE, "  Avg. frame jitter: " 
             << double(m_TotalJitter)/(m_NumFrames*1000));
    }
}

void AVGFramerateManager::SetRate(double Rate)
{
    m_Rate = Rate;
    m_NumFrames = 0;
    m_NumRegularFrames = 0;
    m_FramesTooLate = 0;
    m_TimeSpentWaiting = 0;
    m_LastFrameTime = AVGTimeSource::getCurrentTicks();
    m_StartTime = AVGTimeSource::getCurrentTicks();
    m_TotalJitter = 0;
}

double AVGFramerateManager::GetRate()
{
    return m_Rate;
}

void AVGFramerateManager::FrameWait()
{
    m_NumFrames++;
    m_NumRegularFrames++;

    int CurTime = AVGTimeSource::getCurrentTicks();
    int TargetTime = m_LastFrameTime+(int)((1000/m_Rate)*m_NumRegularFrames);
    if (CurTime <= TargetTime) 
    {
        int WaitTime = TargetTime-CurTime;
        if (WaitTime > 200) {
            AVG_TRACE (AVGPlayer::DEBUG_PROFILE, 
                    "FramerateManager: waiting " << TargetTime-CurTime << " ms.");
        }
        m_bLastFrameLate = false;
        AVGTimeSource::get()->sleepUntil(TargetTime);
    }
    else
    {
        m_FramesTooLate++;
        AVG_TRACE(AVGPlayer::DEBUG_PROFILE, "FramerateManager: frame too late by " <<
                CurTime-TargetTime << " ms."); 
        m_NumRegularFrames = 0;
        m_LastFrameTime = AVGTimeSource::getCurrentTicks();
        m_bLastFrameLate = true;
    }
    m_TimeSpentWaiting += AVGTimeSource::getCurrentTicks()-CurTime;
}

void AVGFramerateManager::CheckJitter() {
    int CurTime = AVGTimeSource::getCurrentTicks();
    int TargetTime = m_LastFrameTime+(int)((1000/m_Rate)*m_NumRegularFrames);
    if (CurTime >= TargetTime+2 && !m_bLastFrameLate) {
//        AVG_TRACE(AVGPlayer::DEBUG_PROFILE, "FramerateManager: swap too late by " <<
//                CurTime-TargetTime << " ms."); 
    }
    m_TotalJitter += CurTime - TargetTime;
}

