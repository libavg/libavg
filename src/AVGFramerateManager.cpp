//
// $Id$
//

#include "AVGFramerateManager.h"
#include "AVGTime.h"

#include <sys/time.h>
#include <unistd.h>

#include <iostream>

using namespace std;

AVGFramerateManager::AVGFramerateManager ()
    : m_NumFrames(0),
      m_NumRegularFrames(0),
      m_Rate(0)
{
}

AVGFramerateManager::~AVGFramerateManager ()
{
    if (m_Rate != 0) {
        cerr << "Framerate statistics: " << endl;
        cerr << "  Framerate goal was: " << m_Rate << endl;
        cerr << "  Total frames: " << m_NumFrames << endl;
        cerr << "  Frames too late: " << m_FramesTooLate << endl;
        double TotalTime = double(GetCurrentTicks()-m_StartTime)/1000;
        cerr << "  Total time: " << TotalTime << " seconds" << endl;
        cerr << "  Framerate achieved: " 
             << (m_NumFrames+1)/TotalTime << endl;
        cerr << "  Time spent waiting: " << double (m_TimeSpentWaiting)/1000 
             << " seconds" << endl;
        cerr << "  Percent of time spent waiting: " 
             << double (m_TimeSpentWaiting)/(10*TotalTime) << endl;

    }
}

void AVGFramerateManager::SetRate(int Rate)
{
    m_Rate = Rate;
    m_NumFrames = 0;
    m_NumRegularFrames = 0;
    m_FramesTooLate = 0;
    m_TimeSpentWaiting = 0;
    m_LastFrameTime = GetCurrentTicks();
    m_StartTime = GetCurrentTicks();
}

int AVGFramerateManager::GetRate()
{
    return m_Rate;
}

void AVGFramerateManager::FrameWait()
{
    m_NumFrames++;
    m_NumRegularFrames++;

    int CurTime = GetCurrentTicks();
    int TargetTime = m_LastFrameTime+(int)((1000/(double)m_Rate)*m_NumRegularFrames);
    m_TimeSpentWaiting += TargetTime-CurTime;
    if (CurTime <= TargetTime) 
    {
        int WaitTime = TargetTime-CurTime;
        if (WaitTime > 200) {
            cerr << "FramerateManager warning: waiting " << TargetTime-CurTime << " ms." << endl;
        }
        usleep (WaitTime*1000);
    }
    else
    {
        m_FramesTooLate++;
        //cerr << "FramerateManager warning: frame too late by " << CurTime-TargetTime << " ms." << endl; 
        m_NumRegularFrames = 0;
        m_LastFrameTime = GetCurrentTicks();
    }
}


