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

#include "TUIOInputDevice.h"
#include "TouchEvent.h"
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

TUIOInputDevice::TUIOInputDevice()
    : m_pSocket(0),
      m_LastID(0)
{
}

TUIOInputDevice::~TUIOInputDevice()
{
    if (m_pSocket) {
        m_pSocket->Break();
    }
}

void TUIOInputDevice::start()
{
    string sPort("3333");
    getEnv("AVG_TUIO_PORT", sPort);
    int port;
    try {
        port = stringToInt(sPort);
    } catch (Exception&) {
        throw Exception(AVG_ERR_TYPE, 
                string("TUIO event source: AVG_TUIO_PORT set to invalid value '")
                + sPort + "'");
    }
    MultitouchInputDevice::start();
    try {
        m_pSocket = new UdpListeningReceiveSocket(IpEndpointName(
                IpEndpointName::ANY_ADDRESS, port), this);
    } catch (std::exception &e) {
        throw Exception(AVG_ERR_MT_INIT, 
                string("TUIO event source: Can't initialize networking. ") + e.what());
    }
    if (!m_pSocket->IsBound()) {
        throw Exception(AVG_ERR_MT_INIT, "TUIO event source: Socket not bound.");
    }
    AVG_TRACE(Logger::category::CONFIG,Logger::severity::INFO,
            "TUIO multitouch event source created, listening on port " << port);

#ifndef WIN32
    pthread_create(&m_Thread, NULL, threadFunc, this);
#else
    DWORD threadId;
    m_Thread = CreateThread(0, 0, threadFunc, this, 0, &threadId);
#endif
}

void TUIOInputDevice::ProcessPacket(const char* pData, int size, 
        const IpEndpointName& remoteEndpoint)
{
    lock_guard lock(getMutex());
    try {
        ReceivedPacket packet(pData, size);
        if (packet.IsBundle()) {
            processBundle(ReceivedBundle(packet), remoteEndpoint);
        } else {
            processMessage(ReceivedMessage(packet), remoteEndpoint);
        }
    } catch (osc::Exception& e) {
        AVG_LOG_WARNING("OSC exception: " << e.what());
    }
}

void TUIOInputDevice::processBundle(const ReceivedBundle& bundle, 
        const IpEndpointName& remoteEndpoint) 
{
    try {
        for (ReceivedBundle::const_iterator it = bundle.ElementsBegin(); 
                it != bundle.ElementsEnd(); ++it) 
        {
            if (it->IsBundle()) {
                processBundle(ReceivedBundle(*it), remoteEndpoint);
            } else {
                processMessage(ReceivedMessage(*it), remoteEndpoint);
            }
        }
    } catch (osc::Exception& e) {
        AVG_LOG_WARNING("OSC exception: " << e.what());
    }
}

void TUIOInputDevice::processMessage(const ReceivedMessage& msg, 
        const IpEndpointName& remoteEndpoint) 
{
//    cerr << msg << endl;
    try {
        ReceivedMessageArgumentStream args = msg.ArgumentStream();

        if (strcmp(msg.AddressPattern(), "/tuio/2Dcur") == 0) {
            const char* cmd;
            args >> cmd;

            if (strcmp(cmd,"set")==0) { 
                processSet(args);
            } else if (strcmp(cmd,"alive")==0) {
                processAlive(args);
            } else if (strcmp(cmd, "fseq") == 0 ) {
                int32 fseq;
                args >> fseq;
            } 
        }
    } catch (osc::Exception& e) {
        AVG_LOG_WARNING("Error parsing TUIO message: " << e.what()
                << ". Message was " << msg);
    }
}

void TUIOInputDevice::processSet(ReceivedMessageArgumentStream& args)
{
    osc::int32 tuioID;
    float xpos, ypos;
    float xspeed, yspeed;
    float accel;
    args >> tuioID >> xpos >> ypos >> xspeed >> yspeed >> accel;
    glm::vec2 pos(xpos, ypos);
    glm::vec2 speed(xspeed, yspeed);
//    cerr << "Set: ID: " << tuioID << ", pos: " << pos << ", speed: " << speed 
//        << ", accel: " << accel << endl;
    TouchStatusPtr pTouchStatus = getTouchStatus(tuioID);
    if (!pTouchStatus) {
        // Down
        m_LastID++;
        TouchEventPtr pEvent = createEvent(m_LastID, Event::CURSOR_DOWN, pos, speed); 
        addTouchStatus((long)tuioID, pEvent);
    } else {
        // Move
        TouchEventPtr pEvent = createEvent(0, Event::CURSOR_MOTION, pos, speed); 
        pTouchStatus->pushEvent(pEvent);
    }
}

void TUIOInputDevice::processAlive(ReceivedMessageArgumentStream& args)
{
    m_LiveTUIOIDs.clear();
    int32 tuioID;
    while (!args.Eos()) {
        args >> tuioID;
        m_LiveTUIOIDs.insert(tuioID);
    }

    // Create up events for all ids not in live list.
    set<int> deadTUIOIDs;
    getDeadIDs(m_LiveTUIOIDs, deadTUIOIDs);
    set<int>::iterator it;
    for (it = deadTUIOIDs.begin(); it != deadTUIOIDs.end(); ++it) {
        int id = *it;
        TouchStatusPtr pTouchStatus = getTouchStatus(id);
        TouchEventPtr pOldEvent = pTouchStatus->getLastEvent();
        TouchEventPtr pUpEvent = boost::dynamic_pointer_cast<TouchEvent>(
                pOldEvent->cloneAs(Event::CURSOR_UP));
        pTouchStatus->pushEvent(pUpEvent);
        removeTouchStatus(id);
    }
}

TouchEventPtr TUIOInputDevice::createEvent(int id, Event::Type type, glm::vec2 pos,
        glm::vec2 speed)
{
    const glm::vec2 size = getTouchArea();
    IntPoint screenPos = getScreenPos(pos);
    glm::vec2 screenSpeed(int(speed.x*size.x+0.5), int(speed.y*size.y+0.5));
    TouchEventPtr pEvent(new TouchEvent(id, type, screenPos, Event::TOUCH));
    pEvent->setSpeed(screenSpeed/1000.f);
    return pEvent;
}

}
