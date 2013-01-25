//
//  libavg - Media Playback Engine.
//  Copyright (C) 2003-2011 Ulrich von Zadow
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
#include "UTF8String.h"
#include "ILogHandler.h"

#include <string>
#include <vector>
#include <sstream>
#include <map>


namespace avg {

#ifdef ERROR
#undef ERROR
#endif

namespace logging {
    namespace subsystem {
        const unsigned long NONE=0;
        const unsigned long PROFILE=2;
        const unsigned long PROFILE_VIDEO=8;
        const unsigned long EVENTS=16;
        const unsigned long EVENTS2=32;
        const unsigned long CONFIG=64;
        const unsigned long WARNING=128;
        const unsigned long ERROR=256;
        const unsigned long MEMORY=512;
        const unsigned long APP=1024;
        const unsigned long PLUGIN=2048;
        const unsigned long PLAYER=4096;
        const unsigned long SHADER=8192;
        const unsigned long DEPRECATION=16384;
    }

    namespace level {
        const long CRITICAL = 50;
        const long FATAL = 50;
        const long ERROR = 40; 
        const long WARNING = 30; 
        const long INFO = 20; 
        const long DEBUG = 10; 
        const long NOTSET = 0; 
    }


class AVG_API Logger {
public:
    static Logger* get();
    virtual ~Logger();

    void addLogHandler(LogHandlerPtr logHandler);
    int getCategories() const;
    void setCategories(int flags);
    void pushCategories();
    void popCategories();
    const char * categoryToString(unsigned long category) const;
    int stringToCategory(const std::string& sCategory) const;
    void trace(int category, const UTF8String& sMsg);
    inline bool isFlagSet(int category) const {
        return (category & m_Flags) != 0;
    }
    unsigned long registerCategory(const string cat);

private:
    Logger();
    void setupSubsystem();

    static Logger* m_pLogger;

    int m_Flags;
    std::vector<int> m_FlagStack;
    std::vector<LogHandlerPtr> m_Handlers;
    std::map< unsigned long, string > m_SubsystemToString;
    std::map< const string , unsigned long > m_StringToSubsystem;

    unsigned long m_MaxSubsystemNum;
};

}

#define AVG_TRACE(category, sMsg) { \
if (logging::Logger::get()->isFlagSet(category)) { \
    std::stringstream tmp(std::stringstream::in | std::stringstream::out); \
    tmp << sMsg; \
    logging::Logger::get()->trace(category, tmp.str()); \
    }\
}\

}
#endif
