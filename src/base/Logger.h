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


namespace avg {

#ifdef ERROR
#undef ERROR
#endif

namespace logging {
    namespace subsystem {
        const long NONE=0;
        const long PROFILE=2;
        const long PROFILE_VIDEO=8;
        const long EVENTS=16;
        const long EVENTS2=32;
        const long CONFIG=64;
        const long WARNING=128;
        const long ERROR=256;
        const long MEMORY=512;
        const long APP=1024;
        const long PLUGIN=2048;
        const long PLAYER=4096;
        const long SHADER=8192;
        const long DEPRECATION=16384;
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
    static const char * categoryToString(int category);
    int getCategories() const;
    void setCategories(int flags);
    void pushCategories();
    void popCategories();
    void trace(int category, const UTF8String& sMsg);
    inline bool isFlagSet(int category) {
        return (category & m_Flags) != 0;
    }

private:
    Logger();
    int stringToCategory(const std::string& sCategory) const;

    static Logger* m_pLogger;

    int m_Flags;
    std::vector<int> m_FlagStack;
    std::vector<LogHandlerPtr> m_Handlers;
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
