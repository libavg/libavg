//
// $Id$
//

#include "AVGLogger.h"
#include "IAVGPlayer.h"

#include <sys/time.h>
#include <iomanip>

using namespace std;

AVGLogger* AVGLogger::m_pLogger = 0;

AVGLogger * AVGLogger::get()
{
    if (!m_pLogger) {
        m_pLogger = new AVGLogger;
    }
    return m_pLogger;
}

AVGLogger::AVGLogger()
{
    m_pDest = &cerr;
    m_Flags = IAVGPlayer::DEBUG_ERROR | IAVGPlayer::DEBUG_WARNING;
}

AVGLogger::~AVGLogger()
{
}

void AVGLogger::setDestination(ostream * pDest)
{
    m_pDest = pDest;
}
    
void AVGLogger::setCategories(int flags)
{
    m_Flags = flags | IAVGPlayer::DEBUG_ERROR;
}

void AVGLogger::trace(int category, const std::string& msg)
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
            case IAVGPlayer::DEBUG_BLTS:
                (*m_pDest) << "    BLIT: ";
                break;
            case IAVGPlayer::DEBUG_PROFILE:
                (*m_pDest) << " PROFILE: ";
                break;
            case IAVGPlayer::DEBUG_EVENTS:
            case IAVGPlayer::DEBUG_EVENTS2:
                (*m_pDest) << "  EVENTS: ";
                break;
            case IAVGPlayer::DEBUG_CONFIG:
                (*m_pDest) << "  CONFIG: ";
                break;
            case IAVGPlayer::DEBUG_WARNING:
                (*m_pDest) << " WARNING: ";
                break;
            case IAVGPlayer::DEBUG_ERROR:
                (*m_pDest) << "   ERROR: ";
                break;
        }
        (*m_pDest) << msg << endl;
    }
}


