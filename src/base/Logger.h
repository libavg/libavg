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
#include "Exception.h"

#include <string>
#include <vector>
#include <sstream>
#include <map>

#include <boost/thread.hpp>

#ifdef ERROR
#undef ERROR
#endif

namespace avg {

typedef unsigned severity_t;
typedef size_t category_t;

class AVG_API Logger {
public:
    struct AVG_API severity
    {
        static const severity_t CRITICAL;
        static const severity_t ERROR;
        static const severity_t WARNING;
        static const severity_t INFO;
        static const severity_t DEBUG;
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
        static const category_t LAST_CATEGORY;
    };

    static Logger* get();
    virtual ~Logger();

    static severity_t stringToSeverity(const string& sSeverity);
    static const char * severityToString(const severity_t severity);

    void addLogSink(const LogSinkPtr& logSink);
    void removeLogSink(const LogSinkPtr& logSink);
    category_t getCategories() const;
    void setCategories(category_t flags);
    void pushCategories();
    void popCategories();
    const char * categoryToString(category_t category) const;
    category_t stringToCategory(const std::string& sCategory) const;
    void trace(const UTF8String& sMsg, category_t category, severity_t severity) const;
    category_t registerCategory(const string& cat, severity_t severity=severity::INFO);
    void setSeverity(size_t category, unsigned severity);

    void logDebug(const string& msg, category_t category=category::APP) const;
    void logInfo(const string& msg, category_t category=category::APP) const;
    void logWarning(const string& msg, category_t category=category::APP) const;
    void logError(const string& msg, category_t category=category::APP) const;
    void logCritical(const string& msg, category_t category=category::APP) const;
    void log(const string& msg, category_t category=category::APP,
            severity_t severity=severity::INFO) const;

    inline bool isCategorySet(category_t category) const {
        return (category & m_Flags) != 0;
    }

    inline bool shouldLog(category_t category, severity_t severity) const {
        if ( isCategorySet(category) ){
            std::map<const category_t , const severity_t >::const_iterator it;
            it = m_CategorySeverities.find(category);
            AVG_ASSERT(it != m_CategorySeverities.end());
            return it->second <= severity;
        } else {
            return false;
        }
    }

private:
    Logger();
    void setupCategory();

    size_t m_Flags;
    std::vector<size_t> m_FlagStack;
    std::map< const category_t, const severity_t> m_CategorySeverities;
    std::map< const category_t, string > m_CategoryToString;
    std::map< const string, category_t > m_StringToCategory;

    size_t m_MaxCategoryNum;
    unsigned m_Severity;


    static boost::mutex m_CategoryMutex;
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
