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
      m_Rate(0)
{
}

AVGFramerateManager::~AVGFramerateManager ()
{
    cerr << "Framerate statistics: " << endl;
    cerr << "  Framerate goal was: " << m_Rate << endl;
    cerr << "  Total frames: " << m_NumFrames << endl;
    cerr << "  Total time: " << double(GetCurrentTicks()-m_StartTime)/1000 << " seconds" << endl;

    cerr << "  Framerate achieved: " 
         << (m_NumFrames*1000)/double(GetCurrentTicks()-m_StartTime) << endl;
}

void AVGFramerateManager::SetRate(int Rate)
{
    m_Rate = Rate;
    m_NumFrames = 0;
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

    int CurTime = GetCurrentTicks();
    int TargetTime = m_LastFrameTime+(int)((1000/(double)m_Rate)*m_NumFrames);
    if (CurTime <= TargetTime) 
    {
        int WaitTime = TargetTime-CurTime;
        if (WaitTime > 200) {
            cerr << "FramerateManager warning: waiting " << TargetTime-CurTime << "ms." << endl;
        }
        usleep (WaitTime*1000);
    }
    else
    {
        m_NumFrames = 0;
        m_LastFrameTime = GetCurrentTicks();
    }
}


