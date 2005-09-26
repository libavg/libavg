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
    virtual ~ProfilingZone();
    void clear();
    
    void start();
    void reset();
    long long getUSecs() const;
    long long getAvgUSecs() const;
    const std::string& getName() const;

    // Interface to AVGScopeTimer.
    void add(long long usecs);

private:
    std::string m_sName;
    long long m_TimeSum;
    long long m_AvgTime;
    int m_NumFrames;
    bool m_bIsRegistered;
};

}

#endif
