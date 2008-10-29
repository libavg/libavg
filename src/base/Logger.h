//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2008 Ulrich von Zadow
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  Current versions can be found at www.libavg.de
//

#ifndef _Logger_H_ 
#define _Logger_H_

#include "../api.h"

#include <iostream>
#include <sstream>

namespace avg {

#ifdef ERROR
#undef ERROR
#endif

class AVG_API Logger {
public:
    static Logger* get();
    virtual ~Logger();
   
    void setConsoleDest();
    void setFileDest(const std::string& sFName);
    void setSyslogDest(int facility, int logopt);
    void setCategories(int flags);
    void trace(int category, const std::string& msg);
    inline bool isFlagSet(int category) {
        return (category & m_Flags) != 0;
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
    static const long WATCHDOG;  

    static const long MEMORY;
    static const long APP;
    static const long LOGGER;
    static const long PLUGIN;


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
