//
// $Id$
//

#include "AVGTime.h"
#include "AVGTimeout.h"
#include "AVGException.h"
#include "IJSEvalKruecke.h"

#include <nsIComponentManager.h>

#include <iostream>

using namespace std;

int AVGTimeout::s_LastID = 0;

AVGTimeout::AVGTimeout(int time, string code, bool isInterval)
    : m_Interval(time),
      m_Code(code),
      m_IsInterval(isInterval)
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

void AVGTimeout::Fire(IJSEvalKruecke * pKruecke)
{
    char * pResult;
    pKruecke->CallEval(m_Code.c_str(), &pResult);

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

