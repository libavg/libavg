//
// $Id$
//

#ifndef _Profiler_H_ 
#define _Profiler_H_

#include "ProfilingZone.h"

#include <list>

namespace avg {
    
class Profiler {
public:
    static Profiler& get();
    ~Profiler();
 
    void addZone(ProfilingZone& Zone);
    void clear();
    void start();
    bool isRunning();
    void setActiveZone(ProfilingZone * pZone);
    void dumpFrame();
    void dumpStatistics();
    void reset();


private:
    Profiler();

    typedef std::list<ProfilingZone*> ZoneList;
    ZoneList m_Zones;
    ProfilingZone * m_pActiveZone;
    bool m_bRunning;
};

}

#endif
