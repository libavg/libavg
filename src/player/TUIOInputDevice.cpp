//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2020 Ulrich von Zadow
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

#include "TUIOInputDevice.h"
#include "TouchEvent.h"
#include "TangibleEvent.h"
#include "Player.h"
#include "AVGNode.h"
#include "TouchStatus.h"

#include "../base/Logger.h"
#include "../base/StringHelper.h"
#include "../base/OSHelper.h"
#include "../base/ObjectCounter.h"
#include "../base/Exception.h"

using namespace std;
using namespace osc;

namespace avg {

#ifndef WIN32
void* TUIOInputDevice::threadFunc(void* p)
#else
DWORD WINAPI TUIOInputDevice::threadFunc(LPVOID p)
#endif
{
    TUIOInputDevice* pThis = (TUIOInputDevice*)p;
    pThis->m_pSocket->Run();
    return 0;
};

bool TUIOInputDevice::isEnabled()
{
    string sEnableTUIO;
    getEnv("AVG_ENABLE_TUIO", sEnableTUIO);
    string sDriver;
    getEnv("AVG_MULTITOUCH_DRIVER", sDriver);
    if (sDriver != "") {
        avgDeprecationWarning("1.8", "AVG_MULTITOUCH_DRIVER", "AVG_ENABLE_TUIO");
    }
    return (sDriver == "TUIO" || sEnableTUIO != "");
}

TUIOInputDevice::TUIOInputDevice(const DivNodePtr& pEventReceiverNode, int port)
    : MultitouchInputDevice(pEventReceiverNode),
      m_pSocket(0),
      m_RemoteIP(0),
      m_bConnected(false)
{
    if (port != 0) {
        m_Port = port;
    } else {
        string sPort("3333");
        getEnv("AVG_TUIO_PORT", sPort);
        try {
            m_Port = stringToInt(sPort);
        } catch (Exception&) {
            throw Exception(AVG_ERR_TYPE, 
                    string("TUIO event source: AVG_TUIO_PORT set to invalid value '")
                    + sPort + "'");
        }
    }

    try {
        m_pSocket = new UdpListeningReceiveSocket(IpEndpointName(
                IpEndpointName::ANY_ADDRESS, m_Port), this);
    } catch (std::exception &e) {
        throw Exception(AVG_ERR_MT_INIT, 
                string("TUIO event source (port ") + toString(m_Port) + "). " + e.what());
    }
    if (!m_pSocket->IsBound()) {
        throw Exception(AVG_ERR_MT_INIT, "TUIO event source: Socket not bound.");
    }
    AVG_TRACE(Logger::category::CONFIG,Logger::severity::INFO,
            "TUIO multitouch event source created, listening on port " << m_Port);

#ifndef WIN32
    pthread_create(&m_Thread, NULL, threadFunc, this);
#else
    DWORD threadId;
    m_Thread = CreateThread(0, 0, threadFunc, this, 0, &threadId);
#endif
}

TUIOInputDevice::~TUIOInputDevice()
{
    if (m_pSocket) {
        m_pSocket->AsynchronousBreak();
    }
#ifndef WIN32
    pthread_join(m_Thread, 0);
#else
    WaitForSingleObject(m_Thread, INFINITE);
#endif
    delete m_pSocket;
}

unsigned TUIOInputDevice::getRemoteIP() const
{
    return m_RemoteIP;
}

BitmapPtr TUIOInputDevice::getUserBmp() const
{
    return m_pUserBmp;
}

void TUIOInputDevice::ProcessPacket(const char* pData, int size, 
        const IpEndpointName& remoteEndpoint)
{
    lock_guard lock(getMutex());
    m_RemoteIP = remoteEndpoint.address;
    try {
        ReceivedPacket packet(pData, size);
        if (packet.IsBundle()) {
            processBundle(ReceivedBundle(packet));
        } else {
            processMessage(ReceivedMessage(packet));
        }
    } catch (osc::Exception& e) {
        AVG_LOG_WARNING("OSC exception: " << e.what());
    }
}

void TUIOInputDevice::processBundle(const ReceivedBundle& bundle) 
{
    try {
        for (ReceivedBundle::const_iterator it = bundle.ElementsBegin(); 
                it != bundle.ElementsEnd(); ++it) 
        {
            if (it->IsBundle()) {
                processBundle(ReceivedBundle(*it));
            } else {
                processMessage(ReceivedMessage(*it));
            }
        }
    } catch (osc::Exception& e) {
        AVG_LOG_WARNING("OSC exception: " << e.what());
    }
}

void TUIOInputDevice::processMessage(const ReceivedMessage& msg) 
{
    try {
        ReceivedMessageArgumentStream args = msg.ArgumentStream();
        const char* cmd;
        args >> cmd;
        if (strcmp(msg.AddressPattern(), "/tuio/2Dcur") == 0) {
            if (strcmp(cmd, "set") == 0) { 
                processTouchSet(args);
            } else if (strcmp(cmd, "alive") == 0) {
                processAlive(args, Event::TOUCH);
            } 
        } else if (strcmp(msg.AddressPattern(), "/tuio/2Dobj") == 0) {
            if (strcmp(cmd, "set") == 0) { 
                processTangibleSet(args);
            } else if (strcmp(cmd, "alive") == 0) {
                processAlive(args, Event::TANGIBLE);
            } 
        } else if (strcmp(msg.AddressPattern(), "/tuioext/userid") == 0) {
            if (strcmp(cmd, "set") == 0) { 
                processUserID(args);
            } else if (strcmp(cmd, "indexframe") == 0) {
                processIndexFrame(args);
            }
        }
        if (!m_bConnected && strstr(msg.AddressPattern(), "/tuio") != 0) {
            m_bConnected = true;
            AVG_TRACE(Logger::category::CONFIG, Logger::severity::INFO,
                    "Receiving TUIO messages");
        }
    } catch (osc::Exception& e) {
        AVG_LOG_WARNING("Error parsing TUIO message: " << e.what()
                << ". Message was " << msg);
    }
}

void TUIOInputDevice::processTouchSet(ReceivedMessageArgumentStream& args)
{
    osc::int32 tuioID;
    float xpos, ypos;
    float xspeed, yspeed;
    float accel;
    args >> tuioID >> xpos >> ypos >> xspeed >> yspeed >> accel;
    glm::vec2 pos(xpos, ypos);
    glm::vec2 speed(xspeed, yspeed);
    TouchStatusPtr pTouchStatus = getTouchStatus(tuioID);
    IntPoint screenPos = getScreenPos(pos);
    TouchEventPtr pEvent;
    if (!pTouchStatus) {
        // Down
        pEvent = TouchEventPtr(new TouchEvent(getNextContactID(), Event::CURSOR_DOWN, 
                    screenPos, Event::TOUCH));
        addTouchStatus((long)tuioID, pEvent);
    } else {
        // Move
        pEvent = TouchEventPtr(new TouchEvent(0, Event::CURSOR_MOTION, screenPos, 
                    Event::TOUCH));
        pTouchStatus->pushEvent(pEvent);
    }
    setEventSpeed(pEvent, speed);
}

void TUIOInputDevice::processTangibleSet(ReceivedMessageArgumentStream& args)
{
    osc::int32 tuioID;
    osc::int32 classID;
    float xpos, ypos;
    float angle;
    float xspeed, yspeed;
    float angleSpeed;
    float accel;
    float angleAccel;
    args >> tuioID >> classID >> xpos >> ypos >> angle >> xspeed >> yspeed >> angleSpeed
            >> accel >> angleAccel;
    glm::vec2 pos(xpos, ypos);
    glm::vec2 speed(xspeed, yspeed);
    TouchStatusPtr pTouchStatus = getTouchStatus(tuioID);
    IntPoint screenPos = getScreenPos(pos);
    TangibleEventPtr pEvent;
    if (!pTouchStatus) {
        // Down
        pEvent = TangibleEventPtr(new TangibleEvent(getNextContactID(), classID, 
                Event::CURSOR_DOWN, screenPos, speed, angle));
        addTouchStatus((long)tuioID, pEvent);
    } else {
        // Move
        pEvent = TangibleEventPtr(new TangibleEvent(0, classID, Event::CURSOR_MOTION, 
                screenPos, speed, angle));
        pTouchStatus->pushEvent(pEvent);
    }
    setEventSpeed(pEvent, speed);
}

void TUIOInputDevice::processAlive(ReceivedMessageArgumentStream& args, 
        Event::Source source)
{
    std::set<int> liveTUIOIDs;
    int32 tuioID;
    while (!args.Eos()) {
        args >> tuioID;
        liveTUIOIDs.insert(tuioID);
    }

    // Create up events for all ids not in live list.
    set<int> deadTUIOIDs;
    getDeadIDs(liveTUIOIDs, deadTUIOIDs, source);
    set<int>::iterator it;
    for (it = deadTUIOIDs.begin(); it != deadTUIOIDs.end(); ++it) {
        int id = *it;
        TouchStatusPtr pTouchStatus = getTouchStatus(id);
        CursorEventPtr pOldEvent = pTouchStatus->getLastEvent();
        CursorEventPtr pUpEvent = pOldEvent->cloneAs(Event::CURSOR_UP);
        pTouchStatus->pushEvent(pUpEvent);
        removeTouchStatus(id);
    }
}

void TUIOInputDevice::processUserID(ReceivedMessageArgumentStream& args)
{
    osc::int32 tuioID;
    osc::int32 userID;
    osc::int32 jointID;
    args >> tuioID >> userID >> jointID;
    TouchStatusPtr pTouchStatus = getTouchStatus(tuioID);
    if (!pTouchStatus) {
        AVG_TRACE(Logger::category::EVENTS, Logger::severity::WARNING,
                "Received /tuioext/userid, but tuio id " << tuioID <<
                " doesn't correspond to a contact.");
        return;
    }
    CursorEventPtr pEvent = pTouchStatus->getLastEvent();
    pEvent->setUserID(userID, jointID);
}

void TUIOInputDevice::processIndexFrame(osc::ReceivedMessageArgumentStream& args)
{
    osc::int32 xsize;
    osc::int32 ysize;
    osc::Blob blob;
    args >> xsize >> ysize >> blob >> osc::EndMessage;
    m_pUserBmp = BitmapPtr(new Bitmap(IntPoint(xsize,ysize), I8,
            (unsigned char*)blob.data, xsize, true));
}

void TUIOInputDevice::setEventSpeed(CursorEventPtr pEvent, glm::vec2 speed)
{
    const glm::vec2 size = getTouchArea();
    glm::vec2 screenSpeed(int(speed.x*size.x+0.5), int(speed.y*size.y+0.5));
    pEvent->setSpeed(screenSpeed/1000.f);
}

void TUIOInputDevice::getDeadIDs(const set<int>& liveIDs, set<int>& deadIDs, 
        Event::Source source)
{
    TouchIDMap::const_iterator it;
    const TouchIDMap& touchIDMap = getTouchIDMap();
    for (it = touchIDMap.begin(); it != touchIDMap.end(); ++it) {
        int id = it->first;
        Event::Source curSource = it->second->getLastEvent()->getSource();
        if (curSource == source) {
            set<int>::const_iterator foundIt = liveIDs.find(id);
            if (foundIt == liveIDs.end()) {
                deadIDs.insert(id);
            }
        }
    }
}

}
