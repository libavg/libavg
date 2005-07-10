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

    /**
     * Turns off debug output.
     */
    static const long NONE;
    /**
     * Outputs data about the display subsystem. Useful for timing/performance
     * measurements.
     */
    static const long BLTS;
    /**
     * Outputs performance statistics and frames displayed too late.
     */
    static const long PROFILE;
    static const long PROFILE_LATEFRAMES;
    
    /**
     * Outputs basic event data.
     */
    static const long EVENTS;
    /**
     * Outputs all event data available.
     */
    static const long EVENTS2;
    /**
     * Outputs configuration data.
     */
    static const long CONFIG;  
    /**
     * Outputs warning messages. Default is on.
     */
    static const long WARNING;
    /**
     * Outputs error messages. Can't be shut off.
     */
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
