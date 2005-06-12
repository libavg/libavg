//
// $Id$
//

#include "Logger.h"

#include <sys/time.h>
#include <fstream>
#include <iomanip>

using namespace std;

namespace avg {

Logger* Logger::m_pLogger = 0;

Logger * Logger::get()
{
    if (!m_pLogger) {
        m_pLogger = new Logger;
    }
    return m_pLogger;
}

Logger::Logger()
{
    m_pDest = &cerr;
    m_Flags = ERROR | WARNING | APP;
}

Logger::~Logger()
{
    if (m_pDest != &cerr) {
        delete m_pDest;
    }
}

void Logger::setDestination(const string& sFName)
{
    if (m_pDest != &cerr) {
        delete m_pDest;
    }
    m_pDest = new ofstream(sFName.c_str(), ios::out | ios::app);
    if (!*m_pDest) {
        AVG_TRACE(ERROR, "Could not open " << sFName 
                << " as log destination.");
    } else {
        AVG_TRACE(ERROR, "Logging started ");
    }
}
    
void Logger::setCategories(int flags)
{
    m_Flags = flags | ERROR | APP;
}

void Logger::trace(int category, const std::string& msg)
{
    if (category & m_Flags) {
        struct timeval time;
        gettimeofday(&time, NULL);
        struct tm* pTime;
        pTime = localtime(&time.tv_sec);
        char timeString[256];
        strftime(timeString, sizeof(timeString), "%y-%m-%d %H:%M:%S", pTime);
        
        (*m_pDest) << "[" << timeString << "." << 
                setw(3) << setfill('0') << time.tv_usec/1000 << setw(0) << "] "; 
        switch(category) {
            case BLTS:
                (*m_pDest) << "    BLIT: ";
                break;
            case PROFILE:
            case PROFILE_LATEFRAMES:
                (*m_pDest) << " PROFILE: ";
                break;
            case EVENTS:
            case EVENTS2:
                (*m_pDest) << "  EVENTS: ";
                break;
            case CONFIG:
                (*m_pDest) << "  CONFIG: ";
                break;
            case WARNING:
                (*m_pDest) << " WARNING: ";
                break;
            case ERROR:
                (*m_pDest) << "   ERROR: ";
                break;
            case APP:
                (*m_pDest) << "     APP: ";
                break;
        }
        (*m_pDest) << msg << endl;
    }
}

void Logger::trace(int category, const char * msg)
{
    trace(category, (std::string&)msg);
}

}
