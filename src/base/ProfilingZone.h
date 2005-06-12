//
// $Id$
//

#ifndef _ProfilingZone_H_ 
#define _ProfilingZone_H_

#include "TimeSource.h"

#include <string>

namespace avg {

class ProfilingZone {
public:
    ProfilingZone(const std::string& sName);
    ~ProfilingZone();
    void clear();
    
    void start();
    void reset();
    CycleCount getUSecs() const;
    CycleCount getAvgUSecs() const;
    const std::string& getName() const;

    // Interface to AVGScopeTimer.
    void addCycles(CycleCount Cycles);

private:
    std::string m_sName;
    CycleCount m_TimeSum;
    CycleCount m_AvgTime;
    int m_NumFrames;
    bool m_bIsRegistered;
};

}

#endif
