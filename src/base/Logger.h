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
   
    void setConsoleDest();
    void setFileDest(const std::string& sFName);
    void setSyslogDest(int facility, int logopt);
    void setCategories(int flags);
    void trace(int category, const std::string& msg);
    inline bool isFlagSet(int category) {
        return (category & m_Flags);
    }

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
    static const long LOGGER;

private:
    Logger();
    static const char * categoryToString(int category);
    void closeDest();
   
    static Logger* m_pLogger;

    enum DestType {CONSOLE, FILE, SYSLOG};
    DestType m_DestType;
    
    std::ostream * m_pDest; // For console and file
    
    int m_Flags;
};

#define AVG_TRACE(category, msg) { \
    if (Logger::get()->isFlagSet(category)) { \
        std::stringstream tmp(std::stringstream::in | std::stringstream::out); \
        tmp << msg; \
        Logger::get()->trace(category, tmp.str()); \
    }\
}

}
#endif
