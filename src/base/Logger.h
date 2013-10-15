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

#include "Exception.h"
#include "ILogSink.h"
#include "UTF8String.h"
#include "../api.h"

#include <boost/thread.hpp>
#include <boost/thread/locks.hpp>
#include <boost/noncopyable.hpp>
#include <boost/functional/hash.hpp>

#include <string>
#include <vector>
#include <sstream>
#include <map>

#ifdef ERROR
#undef ERROR
#endif

namespace avg {

typedef std::map< const category_t, const severity_t > CatToSeverityMap;
typedef std::map< const size_t, const severity_t > CatHashToSeverityMap;

#ifdef _WIN32
// non dll-interface class used as base for dll-interface class
#pragma warning(disable:4275) 
#endif
class AVG_API Logger: private boost::noncopyable {
public:
    struct AVG_API severity
    {
        static const severity_t CRITICAL;
        static const severity_t ERROR;
        static const severity_t WARNING;
        static const severity_t INFO;
        static const severity_t DEBUG;
        static const severity_t NONE;
    };

    struct AVG_API category
    {
        static const category_t NONE;
        static const category_t PROFILE;
        static const category_t PROFILE_VIDEO;
        static const category_t EVENTS;
        static const category_t CONFIG;
        static const category_t MEMORY;
        static const category_t APP;
        static const category_t PLUGIN;
        static const category_t PLAYER;
        static const category_t SHADER;
        static const category_t DEPRECATION;
    };

    static Logger* get();
    virtual ~Logger();

    static severity_t stringToSeverity(const string& sSeverity);
    static const char * severityToString(const severity_t severity);

    void addLogSink(const LogSinkPtr& logSink);
    void removeLogSink(const LogSinkPtr& logSink);
    void removeStdLogSink();

    category_t configureCategory(category_t category,
            severity_t severity=severity::NONE);
    CatToSeverityMap getCategories();

    void trace(const UTF8String& sMsg, const category_t& category,
            severity_t severity) const;
    void logDebug(const UTF8String& msg,
            const category_t& category=category::APP) const;
    void logInfo(const UTF8String& msg,
            const category_t& category=category::APP) const;
    void logWarning(const UTF8String& msg,
            const category_t& category=category::APP) const;
    void logError(const UTF8String& msg,
            const category_t& category=category::APP) const;
    void logCritical(const UTF8String& msg,
            const category_t& category=category::APP) const;
    void log(const UTF8String& msg, const category_t& category=category::APP,
            severity_t severity=severity::INFO) const;

    inline bool shouldLog(const category_t& category, severity_t severity) const {
        boost::lock_guard<boost::mutex> lock(m_CategoryMutex);
        const size_t hashCat = makeHash(category);
        CatHashToSeverityMap::const_iterator it;
        it = m_CategoryHashSeverities.find(hashCat);
        if(m_CategoryHashSeverities.end() != it) {
            return it->second <= severity;
        } else {
            string msg("Unknown category: " + category);
            throw Exception(AVG_ERR_INVALID_ARGS, msg);
        }
    }

private:
    Logger();
    void setupCategory();

    std::vector<LogSinkPtr> m_pSinks;
    LogSinkPtr m_pStdSink;
    CatToSeverityMap m_CategorySeverities;
    CatHashToSeverityMap m_CategoryHashSeverities;
    severity_t m_Severity;
    static boost::mutex m_CategoryMutex;
    boost::hash<UTF8String> makeHash;
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
