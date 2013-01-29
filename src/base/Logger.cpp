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

#include "OSHelper.h"
#include "Exception.h"
#include "StandardLoggingHandler.h"

#ifdef _WIN32
#include <Winsock2.h>
#undef ERROR
#undef WARNING
#include <time.h>
#include <Mmsystem.h>
#else
#include <sys/time.h>
#include <syslog.h>
#endif
#include <iostream>
#include <iomanip>
#include <boost/thread.hpp>
#include <boost/algorithm/string.hpp>

using namespace std;

namespace avg {
namespace logging{

Logger* Logger::m_pLogger = 0;
boost::mutex logMutex;

long levelToLong(const string& sLevel){
    string level = boost::to_upper_copy(sLevel);
    if (level == "CRITICAL"){
        return level::CRITICAL;
    }else if (level == "FATAL"){
        return level::FATAL;
    }else if (level == "ERROR"){
        return level::ERROR;
    }else if (level == "WARNING"){
        return level::WARNING;
    }else if (level == "INFO"){
        return level::INFO;
    }else if (level == "DEBUG"){
        return level::DEBUG;
    }else if (level == "NOTSET"){
        return level::NOTSET;
    }
    throw Exception(AVG_ERR_INVALID_ARGS, level + " is an invalid log level");
}

Logger * Logger::get()
{
    if (!m_pLogger) {
        boost::mutex::scoped_lock lock(logMutex);
        m_pLogger = new Logger;
        m_pLogger->addLogHandler(LogHandlerPtr(new StandardLoggingHandler()));
    }
    return m_pLogger;
}

Logger::Logger()
{
    setupCategory();
    m_Level = level::WARNING;
    string sEnvLevel;
    bool bEnvLevelSet = getEnv("AVG_LOG_LEVEL", sEnvLevel);
    if(bEnvLevelSet){
        m_Level = levelToLong(sEnvLevel);
    }
    m_Flags = category::NONE | category::APP | category::DEPRECATION;
    string sEnvCategories;
    bool bEnvSet = getEnv("AVG_LOG_CATEGORIES", sEnvCategories);
    if (bEnvSet) {
        m_Flags = category::NONE;
        bool bDone = false;
        string sCategory;
        do {
            string::size_type pos = sEnvCategories.find(":");
            if (pos == string::npos) {
                sCategory = sEnvCategories;
                bDone = true;
            } else {
                sCategory = sEnvCategories.substr(0, pos);
                sEnvCategories = sEnvCategories.substr(pos+1);
            }
            size_t category = stringToCategory(sCategory);
            m_Flags |= category;
            } while (!bDone);
    }
    m_MaxCategoryNum = category::DEPRECATION;
}

Logger::~Logger()
{
}

size_t Logger::getCategories() const
{
    boost::mutex::scoped_lock lock(logMutex);
    return m_Flags;
}

void Logger::setCategories(size_t flags)
{
    boost::mutex::scoped_lock lock(logMutex);
    m_Flags = flags;
}

void Logger::pushCategories()
{
    boost::mutex::scoped_lock lock(logMutex);
    m_FlagStack.push_back(m_Flags);
}

void Logger::popCategories()
{
    boost::mutex::scoped_lock lock(logMutex);
    if (m_FlagStack.empty()) {
        throw Exception(AVG_ERR_OUT_OF_RANGE, "popCategories: Nothing to pop.");
    }
    m_Flags = m_FlagStack.back();
    m_FlagStack.pop_back();
}

void Logger::addLogHandler(LogHandlerPtr logHandler)
{
    m_Handlers.push_back(logHandler);
}

void Logger::trace(const UTF8String& sMsg, size_t category, long level) const
{
    boost::mutex::scoped_lock lock(logMutex);
    if (category & m_Flags && m_Level <= level) {
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
        for(size_t i=0; i < m_Handlers.size(); ++i){
            m_Handlers.at(i)->logMessage(pTime, millis, category, sMsg);
        }
    }
}

void Logger::logDebug(const string& msg, const size_t category) const
{
    trace(msg, category, level::DEBUG);
}

void Logger::logInfo(const string& msg, const size_t category) const
{
    trace(msg, category, level::INFO);
}

void Logger::logWarning(const string& msg, const size_t category) const
{
    trace(msg, category, level::WARNING);
}

void Logger::logError(const string& msg, const size_t category) const
{
    trace(msg, category, level::ERROR);
}

void Logger::logCritical(const string& msg, const size_t category) const
{
    trace(msg, category, level::CRITICAL);
}

void Logger::log(const string& msg, const size_t category) const
{
    trace(msg, category, level::INFO);
}

const char * Logger::categoryToString(size_t category) const
{
    std::map< size_t, string >::const_iterator it;
    it = m_CategoryToString.find(category);
    if(it != m_CategoryToString.end()){
        return (it->second).c_str();
    }else{
        return "UNKNOWN";
    }
}

size_t Logger::stringToCategory(const string& sCategory) const
{
    std::map< string , size_t >::const_iterator it;
    it = m_StringToCategory.find(sCategory);
    if(it != m_StringToCategory.end()){
        return it->second;
    }else{
        throw Exception (AVG_ERR_INVALID_ARGS, "Unknown logging category " + sCategory
                + " set using AVG_LOG_CATEGORIES.");
    }
}

void Logger::setupCategory(){
    m_CategoryToString[category::NONE] = "NONE";
    m_StringToCategory["NONE"] = category::NONE;

    m_CategoryToString[category::PROFILE] = "PROFILE",
    m_StringToCategory["PROFILE"] = category::PROFILE;

    m_CategoryToString[category::PROFILE_VIDEO] = "PROFILE_VIDEO";
    m_StringToCategory["PROFILE_VIDEO"] = category::PROFILE_VIDEO;

    m_CategoryToString[category::EVENTS] = "EVENTS";
    m_StringToCategory["EVENTS"] = category::EVENTS;

    m_CategoryToString[category::EVENTS2] = "EVENTS2";
    m_StringToCategory["EVENTS2"] = category::EVENTS2;

    m_CategoryToString[category::CONFIG] = "CONFIG";
    m_StringToCategory["CONFIG"] = category::CONFIG;

    m_CategoryToString[category::MEMORY] = "MEMORY";
    m_StringToCategory["MEMORY"] = category::MEMORY;

    m_CategoryToString[category::APP] = "APP";
    m_StringToCategory["APP"] = category::APP;

    m_CategoryToString[category::PLUGIN] = "PLUGIN";
    m_StringToCategory["PLUGIN"] = category::PLUGIN;

    m_CategoryToString[category::PLAYER] = "PLAYER";
    m_StringToCategory["PLAYER"] = category::PLAYER;

    m_CategoryToString[category::SHADER] = "SHADER";
    m_StringToCategory["SHADER"] = category::SHADER;

    m_CategoryToString[category::DEPRECATION] = "DEPRECATION";
    m_StringToCategory["DEPRECATION"] = category::DEPRECATION;
}

size_t Logger::registerCategory(const string& cat){
    std::map< string, size_t >::iterator it;
    it = m_StringToCategory.find(cat);
    if(it != m_StringToCategory.end()){
        return it->second;
    }else{
        m_MaxCategoryNum *= 2;
        m_CategoryToString[m_MaxCategoryNum] = cat;
        m_StringToCategory[cat] = m_MaxCategoryNum;
        return m_MaxCategoryNum;
    }
}

}
}
