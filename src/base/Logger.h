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
    namespace category {
        const size_t NONE=1; //TODO: In Documentation: NONE doesn't mean no logging, but no category now
        const size_t PROFILE=2;
        const size_t PROFILE_VIDEO=8;
        const size_t EVENTS=16;
        const size_t EVENTS2=32;
        const size_t CONFIG=64;
        const size_t MEMORY=512;
        const size_t APP=1024;
        const size_t PLUGIN=2048;
        const size_t PLAYER=4096;
        const size_t SHADER=8192;
        const size_t DEPRECATION=16384;
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

long levelToLong(const string& sLevel);

class AVG_API Logger {
public:
    static Logger* get();
    virtual ~Logger();

    void addLogHandler(LogHandlerPtr logHandler);
    size_t getCategories() const;
    void setCategories(size_t flags);
    void pushCategories();
    void popCategories();
    const char * categoryToString(size_t category) const;
    size_t stringToCategory(const std::string& sCategory) const;
    void trace(const UTF8String& sMsg, size_t category, long level) const;
    inline bool isFlagSet(size_t category) const {
        return (category & m_Flags) != 0;
    }
    size_t registerCategory(const string& cat);

    void logDebug(const string& msg, const size_t category=category::NONE) const;
    void logInfo(const string& msg, const size_t category=category::NONE) const;
    void logWarning(const string& msg, const size_t category=category::NONE) const;
    void logError(const string& msg, const size_t category=category::NONE) const;
    void logCritical(const string& msg, const size_t category=category::NONE) const;
    void log(const string& msg, const size_t category=category::NONE) const;

    void setLogLevel(long level){
        m_Level = level;
    }

private:
    Logger();
    void setupCategory();

    static Logger* m_pLogger;

    size_t m_Flags;
    std::vector<size_t> m_FlagStack;
    std::vector<LogHandlerPtr> m_Handlers;
    std::map< size_t, string > m_CategoryToString;
    std::map< const string, size_t > m_StringToCategory;

    size_t m_MaxCategoryNum;
    long m_Level;
};

}

#define AVG_TRACE(category, level, sMsg) { \
if (logging::Logger::get()->isFlagSet(category)) { \
    std::stringstream tmp(std::stringstream::in | std::stringstream::out); \
    tmp << sMsg; \
    logging::Logger::get()->trace(tmp.str(), category, level); \
    }\
}\

#define AVG_LOG_ERROR(sMsg){ \
    AVG_TRACE(logging::category::NONE, logging::level::ERROR, sMsg); \
}\

#define AVG_LOG_WARNING(sMsg){ \
    AVG_TRACE(logging::category::NONE, logging::level::WARNING, sMsg); \
}\

#define AVG_LOG_INFO(sMsg){ \
    AVG_TRACE(logging::category::NONE, logging::level::INFO, sMsg); \
}\

}
#endif
