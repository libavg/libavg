//
// $Id$
//

#ifndef _AVGProfiler_H_ 
#define _AVGProfiler_H_

#include "AVGProfilingZone.h"

#include <list>

class AVGProfiler {
public:
    static AVGProfiler& get();
    ~AVGProfiler();
 
    void addZone(AVGProfilingZone& Zone);
    void dump();
    void reset();

private:
    AVGProfiler();

    typedef std::list<AVGProfilingZone*> ZoneList;
    ZoneList m_Zones;

};

#endif
