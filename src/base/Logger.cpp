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

using namespace std;

namespace avg {
namespace logging{

Logger* Logger::m_pLogger = 0;
boost::mutex logMutex;


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
    m_Flags = subsystem::ERROR | subsystem::WARNING | subsystem::APP | subsystem::DEPRECATION;
    string sEnvCategories;
    bool bEnvSet = getEnv("AVG_LOG_CATEGORIES", sEnvCategories);
    if (bEnvSet) {
        m_Flags = subsystem::ERROR | subsystem::APP;
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
            long category = stringToCategory(sCategory);
            m_Flags |= category;
            } while (!bDone);
    }
}

Logger::~Logger()
{
}

int Logger::getCategories() const
{
    boost::mutex::scoped_lock lock(logMutex);
    return m_Flags;
}

void Logger::setCategories(int flags)
{
    boost::mutex::scoped_lock lock(logMutex);
    m_Flags = flags | subsystem::ERROR | subsystem::WARNING;
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

void Logger::addLogHandler(LogHandlerPtr logHandler){
    m_Handlers.push_back(logHandler);
}

void Logger::trace(int category, const UTF8String& sMsg)
{
    boost::mutex::scoped_lock lock(logMutex);
    if (category & m_Flags) {
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
        for(unsigned int i=0; i < m_Handlers.size(); ++i){
            m_Handlers.at(i)->logMessage(pTime, millis, category, sMsg);
        }
    }
}

const char * Logger::categoryToString(int category)
{
    switch(category) {
        case subsystem::PROFILE:
        case subsystem::PROFILE_VIDEO:
            return "PROFILE";
        case subsystem::EVENTS:
        case subsystem::EVENTS2:
            return "EVENTS";
        case subsystem::CONFIG:
            return "CONFIG";
        case subsystem::WARNING:
            return "WARNING";
        case subsystem::ERROR:
            return "ERROR";
        case subsystem::MEMORY:
            return "MEMORY";
        case subsystem::APP:
            return "APP";
        case subsystem::PLUGIN:
            return "PLUGIN";
        case subsystem::PLAYER:
            return "PLAYER";
        case subsystem::SHADER:
            return "SHADER";
        case subsystem::DEPRECATION:
            return "DEPRECATION";
        default:
            return "UNKNOWN";
    }
}

int Logger::stringToCategory(const string& sCategory) const
{
    if (sCategory == "PROFILE") {
        return subsystem::PROFILE;
    } else if (sCategory == "PROFILE_VIDEO") {
        return subsystem::PROFILE_VIDEO;
    } else if (sCategory == "EVENTS") {
        return subsystem::EVENTS;
    } else if (sCategory == "EVENTS2") {
        return subsystem::EVENTS2;
    } else if (sCategory == "CONFIG") {
        return subsystem::CONFIG;
    } else if (sCategory == "WARNING") {
        return subsystem::WARNING;
    } else if (sCategory == "ERROR") {
        return subsystem::ERROR;
    } else if (sCategory == "MEMORY") {
        return subsystem::MEMORY;
    } else if (sCategory == "APP") {
        return subsystem::APP;
    } else if (sCategory == "PLUGIN") {
        return subsystem::PLUGIN;
    } else if (sCategory == "PLAYER") {
        return subsystem::PLAYER;
    } else if (sCategory == "SHADER") {
        return subsystem::SHADER;
    } else if (sCategory == "DEPRECATION") {
        return subsystem::DEPRECATION;
    } else {
        throw Exception (AVG_ERR_INVALID_ARGS, "Unknown logging category " + sCategory
                + " set using AVG_LOG_CATEGORIES.");
    }
}

}
}
