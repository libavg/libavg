//
// $Id$
//

#include "AVGProfilingZone.h"
#include "AVGProfiler.h"

using namespace std;

AVGProfilingZone::AVGProfilingZone(const string& sName)
    : m_sName(sName),
      m_TimeSum(0),
      m_AvgTime(0),
      m_NumFrames(0)
{
    AVGProfiler::get().addZone(*this);
}

AVGProfilingZone::~AVGProfilingZone() 
{
}

void AVGProfilingZone::reset()
{
    m_NumFrames++;
    m_AvgTime = (m_AvgTime*(m_NumFrames-1)+m_TimeSum)/m_NumFrames;
    m_TimeSum = 0;
}

CycleCount AVGProfilingZone::getAvgCycles() const
{
    return m_AvgTime;
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


