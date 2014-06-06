//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2014 Ulrich von Zadow
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

#ifndef _TUIOInputDevice_H_
#define _TUIOInputDevice_H_

#include "../api.h"
#include "MultitouchInputDevice.h"
#include "Event.h"
#include "../oscpack/UdpSocket.h"
#include "../oscpack/PacketListener.h"
#include "../oscpack/OscReceivedElements.h"
#include "../oscpack/OscPrintReceivedElements.h"

#ifdef WIN32
#include <windows.h>
#endif

#include <set>

namespace avg {

class AVG_API TUIOInputDevice: public MultitouchInputDevice, PacketListener
{
public:
    TUIOInputDevice(const DivNodePtr& pEventReceiverNode=DivNodePtr(), int port=0);
    virtual ~TUIOInputDevice();
    virtual void start();
   
    virtual unsigned getRemoteIP() const;

    virtual void ProcessPacket(const char* pData, int size, 
            const IpEndpointName& remoteEndpoint);
    
    virtual void processMessage(const osc::ReceivedMessage& msg);

private:
#ifndef WIN32
    static void* threadFunc(void* p);
#else
    static DWORD WINAPI threadFunc(LPVOID p);
#endif
    void processBundle(const osc::ReceivedBundle& bundle);
    void processTouchSet(osc::ReceivedMessageArgumentStream& args);
    void processTangibleSet(osc::ReceivedMessageArgumentStream& args);
    void processAlive(osc::ReceivedMessageArgumentStream& args, 
            Event::Source source);
    void setEventSpeed(CursorEventPtr pEvent, glm::vec2 speed);
    void getDeadIDs(const std::set<int>& liveIDs, std::set<int>& deadIDs, 
            Event::Source source);

    UdpListeningReceiveSocket* m_pSocket;
    unsigned m_RemoteIP;
    int m_Port;
#ifndef WIN32
    pthread_t m_Thread;
#else
    HANDLE m_Thread;
#endif  
};

typedef boost::shared_ptr<TUIOInputDevice> TUIOInputDevicePtr;

}

#endif

