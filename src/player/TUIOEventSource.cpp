//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2008 Ulrich von Zadow
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

#include "TUIOEventSource.h"
#include "TouchEvent.h"
#include "Player.h"
#include "AVGNode.h"
#include "TouchStatus.h"

#include "../base/Logger.h"
#include "../base/StringHelper.h"
#include "../base/OSHelper.h"
#include "../base/Point.h"
#include "../base/ObjectCounter.h"
#include "../base/Exception.h"

using namespace std;
using namespace osc;

namespace avg {

#ifndef WIN32
void* TUIOEventSource::threadFunc(void* p)
#else
DWORD WINAPI TUIOEventSource::threadFunc(LPVOID p)
#endif
{
    TUIOEventSource* pThis = (TUIOEventSource*)p;
    pThis->m_pSocket->Run();
    return 0;
};

TUIOEventSource::TUIOEventSource()
    : m_LastID(0)
{
}

TUIOEventSource::~TUIOEventSource()
{
    if (m_pSocket) {
        m_pSocket->Break();
    }
}

void TUIOEventSource::start()
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
    MultitouchEventSource::start();
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
    AVG_TRACE(Logger::CONFIG, "TUIO multitouch event source created, listening on port "
            << port);

#ifndef WIN32
    pthread_create(&m_Thread, NULL, threadFunc, this);
#else
    DWORD threadId;
    m_Thread = CreateThread(0, 0, threadFunc, this, 0, &threadId);
#endif
}

void TUIOEventSource::ProcessPacket(const char* pData, int size, 
        const IpEndpointName& remoteEndpoint)
{
    boost::mutex::scoped_lock lock(getMutex());
    try {
        ReceivedPacket packet(pData, size);
        if (packet.IsBundle()) {
            processBundle(ReceivedBundle(packet), remoteEndpoint);
        } else {
            processMessage(ReceivedMessage(packet), remoteEndpoint);
        }
    } catch (MalformedBundleException& e) {
        AVG_TRACE(Logger::WARNING, "Malformed OSC bundle received: " << e.what());
    }
}

void TUIOEventSource::processBundle(const ReceivedBundle& bundle, 
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
    } catch (MalformedBundleException& e) {
        AVG_TRACE(Logger::WARNING, "Malformed OSC bundle received: " << e.what());
    }
}

void TUIOEventSource::processMessage(const ReceivedMessage& msg, 
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
        AVG_TRACE(Logger::WARNING, "Error parsing TUIO message: " << e.what()
                << ". Message was " << msg);
    }
}

void TUIOEventSource::processSet(ReceivedMessageArgumentStream& args)
{
    osc::int32 tuioID;
    float xpos, ypos;
    float xspeed, yspeed;
    float accel;
    args >> tuioID >> xpos >> ypos >> xspeed >> yspeed >> accel;
    DPoint pos(xpos, ypos);
    DPoint speed(xspeed, yspeed);
//    cerr << "Set: ID: " << tuioID << ", pos: " << pos << ", speed: " << speed 
//        << ", accel: " << accel << endl;
    TouchStatusPtr pTouchStatus = getTouchStatus(tuioID);
    if (!pTouchStatus) {
        // Down
        m_LastID++;
        TouchEventPtr pEvent = createEvent(m_LastID, Event::CURSORDOWN, pos, speed); 
        addTouchStatus((long)tuioID, pEvent);
    } else {
        // Move
        TouchEventPtr pEvent = createEvent(0, Event::CURSORMOTION, pos, speed); 
        pTouchStatus->updateEvent(pEvent);
    }
}

void TUIOEventSource::processAlive(ReceivedMessageArgumentStream& args)
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
                pOldEvent->cloneAs(Event::CURSORUP));
        pTouchStatus->updateEvent(pUpEvent);
    }
}

TouchEventPtr TUIOEventSource::createEvent(int id, Event::Type type, DPoint pos,
        DPoint speed)
{
    DPoint size = getWindowSize();
    IntPoint screenPos(int(pos.x*size.x+0.5), int(pos.y*size.y+0.5));
    DPoint screenSpeed(int(speed.x*size.x+0.5), int(speed.y*size.y+0.5));
    return TouchEventPtr(new TouchEvent(id, type, screenPos, Event::TOUCH, screenSpeed, 
            0, 20, 1, DPoint(5,0), DPoint(0,5)));
}

}
