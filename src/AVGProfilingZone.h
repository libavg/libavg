//
// $Id$
//

#ifndef _AVGProfilingZone_H_ 
#define _AVGProfilingZone_H_

#include "AVGTimeSource.h"

#include <string>

class AVGProfilingZone {
public:
    AVGProfilingZone(const std::string& sName);
    ~AVGProfilingZone();
 
    void reset();
    CycleCount getCycles() const;
    CycleCount getAvgCycles() const;
    const std::string& getName() const;

    // Interface to AVGScopeTimer.
    void addCycles(CycleCount Cycles);

private:
    std::string m_sName;
    CycleCount m_TimeSum;
    CycleCount m_AvgTime;
    int m_NumFrames;
};

#endif
