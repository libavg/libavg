//
// $Id$
//

#include "Timeout.h"

#include "../base/TimeSource.h"
#include "../base/Exception.h"

#include <iostream>

using namespace std;

namespace avg {

int Timeout::s_LastID = 0;

Timeout::Timeout(int time, string code, bool isInterval, JSContext * pContext)
    : m_Interval(time),
      m_IsInterval(isInterval),
      m_Script(code, "timeout", 0, pContext)
{
    m_NextTimeout = m_Interval+TimeSource::get()->getCurrentTicks();
//    cerr << "New timeout. m_Interval=" << m_Interval << ", m_NextTimeout="
//            << m_NextTimeout << endl;
    s_LastID++;
    m_ID = s_LastID;
}

Timeout::~Timeout()
{
}

bool Timeout::IsReady() const
{
    return m_NextTimeout <= TimeSource::get()->getCurrentTicks();
}

bool Timeout::IsInterval() const
{
    return m_IsInterval;
}

void Timeout::Fire(JSContext * pJSContext)
{
    m_Script.run();
    if (m_IsInterval) {
        m_NextTimeout = m_Interval + TimeSource::get()->getCurrentTicks();
//        cerr << "Interval::Fire. m_Interval=" << m_Interval << ", m_NextTimeout="
//            << m_NextTimeout << endl;
    }
}

int Timeout::GetID() const
{
    return m_ID;
}

bool Timeout::operator <(const Timeout& other) const
{
    return m_NextTimeout < other.m_NextTimeout;
}

}
