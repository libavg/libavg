//
// $Id$
//

#include "AVGTime.h"
#include "AVGTimeout.h"
#include "AVGException.h"

#include <nsIComponentManager.h>
#include <nsMemory.h>

#include <iostream>

using namespace std;

int AVGTimeout::s_LastID = 0;

AVGTimeout::AVGTimeout(int time, string code, bool isInterval, JSContext * pContext)
    : m_Interval(time),
      m_IsInterval(isInterval),
      m_Script(code, "timeout", 0, pContext)
{
    m_NextTimeout = m_Interval+GetCurrentTicks();
    s_LastID++;
    m_ID = s_LastID;
}

AVGTimeout::~AVGTimeout()
{
}

bool AVGTimeout::IsReady() const
{
    return m_NextTimeout <= (int)GetCurrentTicks();
}

bool AVGTimeout::IsInterval() const
{
    return m_IsInterval;
}

void AVGTimeout::Fire(JSContext * pJSContext)
{
    m_Script.run();

    if (m_IsInterval) {
        m_NextTimeout = m_Interval + GetCurrentTicks();
    }
}

int AVGTimeout::GetID() const
{
    return m_ID;
}

bool AVGTimeout::operator <(const AVGTimeout& other) const
{
    return m_NextTimeout < other.m_NextTimeout;
}

