//
// $Id$
//

#include "AVGProfilingZone.h"
#include "AVGProfiler.h"

using namespace std;

AVGProfilingZone::AVGProfilingZone(const string& sName)
    : m_sName(sName),
      m_TimeSum(0)
{
    AVGProfiler::get().addZone(*this);
}

AVGProfilingZone::~AVGProfilingZone() 
{
}

void AVGProfilingZone::reset()
{
    m_TimeSum = 0;
}

CycleCount AVGProfilingZone::getCycles() const
{
    return m_TimeSum;
}

const std::string& AVGProfilingZone::getName() const
{
    return m_sName;
}


void AVGProfilingZone::addCycles(CycleCount Cycles)
{
    m_TimeSum += Cycles;
}


