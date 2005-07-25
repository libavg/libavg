//
// $Id$
//

#ifndef _Logger_H_ 
#define _Logger_H_

#include <iostream>
#include <sstream>

namespace avg {

class Logger {
public:
    static Logger* get();
    virtual ~Logger();
   
    void setDestination(const std::string& sFName);
    void setCategories(int flags);
    void trace(int category, const std::string& msg);

    static const long NONE;
    static const long BLTS;
    static const long PROFILE;
    static const long PROFILE_LATEFRAMES;
    static const long EVENTS;
    static const long EVENTS2;
    static const long CONFIG;  
    static const long WARNING;
    static const long ERROR;  

    static const long MEMORY;
    static const long APP;

private:
    Logger();
   
    static Logger* m_pLogger;

    std::ostream * m_pDest;
    int m_Flags;
};

#define AVG_TRACE(category, msg) { \
    std::stringstream tmp(std::stringstream::in | std::stringstream::out); \
    tmp << msg; \
    Logger::get()->trace(category, tmp.str()); \
}

}
#endif
