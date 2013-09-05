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

#include "Logger.h"
#include "Exception.h"
#include "StandardLogSink.h"
#include "OSHelper.h"

#include <boost/algorithm/string.hpp>

#ifdef _WIN32
#include <Winsock2.h>
#include <time.h>
#include <Mmsystem.h>
#undef ERROR
#else
#include <sys/time.h>
#include <syslog.h>
#endif
#include <iostream>
#include <iomanip>

using namespace std;
namespace ba = boost::algorithm;

namespace avg {
    const severity_t Logger::severity::CRITICAL = 50;
    const severity_t Logger::severity::ERROR    = 40;
    const severity_t Logger::severity::WARNING  = 30;
    const severity_t Logger::severity::INFO     = 20;
    const severity_t Logger::severity::DEBUG    = 10;
    const severity_t Logger::severity::NONE  =  0;

    const category_t Logger::category::NONE = UTF8String("NONE");
    const category_t Logger::category::PROFILE = UTF8String("PROFILE");
    const category_t Logger::category::PROFILE_VIDEO = UTF8String("PROFILE_V");
    const category_t Logger::category::EVENTS = UTF8String("EVENTS");
    const category_t Logger::category::CONFIG = UTF8String("CONFIG");
    const category_t Logger::category::MEMORY = UTF8String("MEMORY");
    const category_t Logger::category::APP = UTF8String("APP");
    const category_t Logger::category::PLUGIN = UTF8String("PLUGIN");
    const category_t Logger::category::PLAYER = UTF8String("PLAYER");
    const category_t Logger::category::SHADER = UTF8String("SHADER");
    const category_t Logger::category::DEPRECATION = UTF8String("DEPREC");

namespace {
    Logger* s_pLogger = 0;
    boost::mutex s_logMutex;
    boost::mutex s_traceMutex;
    boost::mutex s_sinkMutex;
    boost::mutex s_removeStdSinkMutex;
}

boost::mutex Logger::m_CategoryMutex;

Logger * Logger::get()
{
    boost::lock_guard<boost::mutex> lock(s_logMutex);
    if (!s_pLogger) {
        s_pLogger = new Logger;
    }
    return s_pLogger;
}

Logger::Logger()
{
    m_Severity = severity::WARNING;
    string sEnvSeverity;
    bool bEnvSeveritySet = getEnv("AVG_LOG_SEVERITY", sEnvSeverity);
    if(bEnvSeveritySet) {
        m_Severity = Logger::stringToSeverity(sEnvSeverity);
    }
    setupCategory();

    string sEnvCategories;
    bool bEnvSet = getEnv("AVG_LOG_CATEGORIES", sEnvCategories);
    if (bEnvSet) {
        vector<string> sCategories;
        ba::split(sCategories, sEnvCategories, ba::is_any_of(" "), ba::token_compress_on);
        vector<string>::iterator it;
        for(it=sCategories.begin(); it!=sCategories.end(); it++) {
            string::size_type pos = (*it).find(":");
            string sCategory;
            string sSeverity = "NONE";
            if(pos == string::npos) {
                sCategory = *it;
            } else {
                vector<string> tmpValues;
                ba::split( tmpValues, *it, ba::is_any_of(":"), ba::token_compress_on);
                sCategory = tmpValues.at(0);
                sSeverity = tmpValues.at(1);
            }
            severity_t severity = stringToSeverity(sSeverity);
            configureCategory(sCategory, severity);
        }
    }

    string sDummy;
    bool bEnvOmitStdErr = getEnv("AVG_LOG_OMIT_STDERR", sDummy);
    if (!bEnvOmitStdErr) {
        m_pStdSink = LogSinkPtr(new StandardLogSink);
        addLogSink(m_pStdSink);
    }
}

Logger::~Logger()
{
}

void Logger::addLogSink(const LogSinkPtr& logSink)
{
    boost::lock_guard<boost::mutex> lock(s_sinkMutex);
    m_pSinks.push_back(logSink);
}

void Logger::removeLogSink(const LogSinkPtr& logSink)
{
    boost::lock_guard<boost::mutex> lock(s_sinkMutex);
    std::vector<LogSinkPtr>::iterator it;
    it = find(m_pSinks.begin(), m_pSinks.end(), logSink);
    if ( it != m_pSinks.end() ) {
        m_pSinks.erase(it);
    }
}

void Logger::removeStdLogSink()
{
    boost::lock_guard<boost::mutex> lock(s_removeStdSinkMutex);
    if ( m_pStdSink.get()) {
        removeLogSink(m_pStdSink);
        m_pStdSink = LogSinkPtr();
    }
}

category_t Logger::configureCategory(category_t category, severity_t severity)
{
    boost::lock_guard<boost::mutex> lock(m_CategoryMutex);
    severity = (severity == Logger::severity::NONE) ? m_Severity : severity;
    UTF8String sCategory = boost::to_upper_copy(string(category));
    const size_t catHash = makeHash(sCategory);
    CatHashToSeverityMap::iterator it;
    it = m_CategoryHashSeverities.find(catHash);
    if ( it != m_CategoryHashSeverities.end()) {
        m_CategoryHashSeverities.erase(it);
        m_CategorySeverities.erase(sCategory);
    }
    pair<const category_t, const severity_t> element(sCategory, severity);
    pair<const size_t, const severity_t> hashedElement(catHash, severity);
    m_CategorySeverities.insert(element);
    m_CategoryHashSeverities.insert(hashedElement);
    return sCategory;
}

CatToSeverityMap Logger::getCategories()
{
    return m_CategorySeverities;
}

void Logger::trace(const UTF8String& sMsg, const category_t& category,
        severity_t severity) const
{
    boost::lock_guard<boost::mutex> lock(s_traceMutex);
    struct tm* pTime;
    #ifdef _WIN32
    __int64 now;
    _time64(&now);
    pTime = _localtime64(&now);
    DWORD tms = timeGetTime();
    unsigned millis = unsigned(tms % 1000);
    #else
    struct timeval time;
    gettimeofday(&time, NULL);
    pTime = localtime(&time.tv_sec);
    unsigned millis = time.tv_usec/1000;
    #endif
    boost::lock_guard<boost::mutex> lockHandler(s_sinkMutex);
    std::vector<LogSinkPtr>::const_iterator it;
    for(it=m_pSinks.begin(); it!=m_pSinks.end(); ++it){
        (*it)->logMessage(pTime, millis, category, severity, sMsg);
    }
}

void Logger::logDebug(const UTF8String& msg, const category_t& category) const
{
    log(msg, category, Logger::severity::DEBUG);
}

void Logger::logInfo(const UTF8String& msg, const category_t& category) const
{
    log(msg, category, Logger::severity::INFO);
}

void Logger::logWarning(const UTF8String& msg, const category_t& category) const
{
    log(msg, category, Logger::severity::WARNING);
}

void Logger::logError(const UTF8String& msg, const category_t& category) const
{
    log(msg, category, Logger::severity::ERROR);
}

void Logger::logCritical(const UTF8String& msg, const category_t& category) const
{
    log(msg, category, Logger::severity::CRITICAL);
}

void Logger::log(const UTF8String& msg, const category_t& category,
        severity_t severity) const
{
    if(shouldLog(category, severity)) {
        Logger::trace(msg, category, severity);
    }
}

void Logger::setupCategory()
{
    configureCategory(category::NONE);
    configureCategory(category::PROFILE);
    configureCategory(category::PROFILE_VIDEO);
    configureCategory(category::EVENTS);
    configureCategory(category::CONFIG);
    configureCategory(category::MEMORY);
    configureCategory(category::APP);
    configureCategory(category::PLUGIN);
    configureCategory(category::PLAYER);
    configureCategory(category::SHADER);
    configureCategory(category::DEPRECATION);
}

severity_t Logger::stringToSeverity(const string& sSeverity)
{
    string severity = boost::to_upper_copy(string(sSeverity));
    if (severity == "CRIT") {
        return Logger::severity::CRITICAL;
    } else if (severity == "ERR") {
        return Logger::severity::ERROR;
    } else if (severity == "WARN") {
        return Logger::severity::WARNING;
    } else if (severity == "INFO") {
        return Logger::severity::INFO;
    } else if (severity == "DBG") {
        return Logger::severity::DEBUG;
    } else if (severity == "NONE") {
        return Logger::severity::NONE;
    }
    throw Exception(AVG_ERR_INVALID_ARGS, severity + " is an invalid log severity");
}

const char * Logger::severityToString(severity_t severity)
{
    if(severity == Logger::severity::CRITICAL) {
        return "CRIT";
    } else if(severity == Logger::severity::ERROR) {
        return "ERR";
    } else if(severity == Logger::severity::WARNING) {
        return "WARN";
    } else if(severity == Logger::severity::INFO) {
        return "INFO";
    } else if(severity == Logger::severity::DEBUG) {
        return "DBG";
    }
    throw Exception(AVG_ERR_UNKNOWN, "Unkown log severity");
}

}
