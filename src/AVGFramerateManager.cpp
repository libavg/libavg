//
// $Id$
//

#include "AVGFramerateManager.h"

#include "SDL/SDL.h"

#include <iostream.h>

using namespace std;

AVGFramerateManager::AVGFramerateManager ()
    : m_NumFrames(0),
      m_Rate(0)
{
}

AVGFramerateManager::~AVGFramerateManager ()
{
}

void AVGFramerateManager::SetRate(int Rate)
{
    m_Rate = Rate;
    m_NumFrames = 0;
    m_LastFrameTime = SDL_GetTicks();
}

int AVGFramerateManager::GetRate()
{
    return m_Rate;
}

void AVGFramerateManager::FrameWait()
{
    m_NumFrames++;

    int CurTime = SDL_GetTicks();
    int TargetTime = m_LastFrameTime+(int)((1000/(double)m_Rate)*m_NumFrames);
    if (CurTime <= TargetTime) 
    {
        if (TargetTime-CurTime > 200) {
            cerr << "FramerateManager warning: waiting " << TargetTime-CurTime << "ms." << endl;
        }
        SDL_Delay(TargetTime-CurTime);
    }
    else
    {
        m_NumFrames = 0;
        m_LastFrameTime = SDL_GetTicks();
    }
}


