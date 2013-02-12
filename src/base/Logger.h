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
#include "ILogSink.h"

#include <string>
#include <vector>
#include <sstream>
#include <map>

#ifdef ERROR
#undef ERROR
#endif

namespace avg {

class AVG_API Logger {
public:
    struct severity
    {
        static const unsigned CRITICAL;
        static const unsigned ERROR;
        static const unsigned WARNING;
        static const unsigned INFO;
        static const unsigned DEBUG;
    };

    struct category
    {
        static const size_t NONE;
        static const size_t PROFILE;
        static const size_t PROFILE_VIDEO;
        static const size_t EVENTS;
        static const size_t EVENTS2;
        static const size_t CONFIG;
        static const size_t MEMORY;
        static const size_t APP;
        static const size_t PLUGIN;
        static const size_t PLAYER;
        static const size_t SHADER;
        static const size_t DEPRECATION;
        static const size_t LAST_CATEGORY;
    };

    static Logger* get();
    virtual ~Logger();

    static unsigned stringToSeverity(const string& sSeverity);
    static const char * severityToString(unsigned severity);

    void addLogSink(const LogSinkPtr& logSink);
    void removeLogSink(const LogSinkPtr& logSink);
    size_t getCategories() const;
    void setCategories(size_t flags);
    void pushCategories();
    void popCategories();
    const char * categoryToString(size_t category) const;
    size_t stringToCategory(const std::string& sCategory) const;
    void trace(const UTF8String& sMsg, size_t category, unsigned severity) const;
    size_t registerCategory(const string& cat);
    void setLogSeverity(unsigned severity);

    void logDebug(const string& msg, const size_t category=category::APP) const;
    void logInfo(const string& msg, const size_t category=category::APP) const;
    void logWarning(const string& msg, const size_t category=category::APP) const;
    void logError(const string& msg, const size_t category=category::APP) const;
    void logCritical(const string& msg, const size_t category=category::APP) const;
    void log(const string& msg, const size_t category=category::APP,
            unsigned severity=severity::INFO) const;

    inline bool isCategorySet(size_t category) const {
        return (category & m_Flags) != 0;
    }

    inline bool shouldLog(size_t category, unsigned severity) const {
        return (m_Severity <= severity && isCategorySet(category)) ||
                Logger::severity::ERROR <= severity;
    }

private:
    Logger();
    void setupCategory();

    size_t m_Flags;
    std::vector<size_t> m_FlagStack;
    std::map< const size_t, string > m_CategoryToString;
    std::map< const string, size_t > m_StringToCategory;

    size_t m_MaxCategoryNum;
    unsigned m_Severity;
};

#define AVG_TRACE(category, severity, sMsg) { \
if (Logger::get()->shouldLog(category, severity)) { \
    std::stringstream tmp(std::stringstream::in | std::stringstream::out); \
    tmp << sMsg; \
    Logger::get()->trace(tmp.str(), category, severity); \
    }\
}\

#define AVG_LOG_ERROR(sMsg){ \
    AVG_TRACE(Logger::category::NONE, Logger::severity::ERROR, sMsg); \
}\

#define AVG_LOG_WARNING(sMsg){ \
    AVG_TRACE(Logger::category::NONE, Logger::severity::WARNING, sMsg); \
}\

#define AVG_LOG_INFO(sMsg){ \
    AVG_TRACE(Logger::category::NONE, Logger::severity::INFO, sMsg); \
}\

#define AVG_LOG_DEBUG(sMsg){ \
    AVG_TRACE(Logger::category::NONE, Logger::severity::DEBUG, sMsg); \
}\

}
#endif
