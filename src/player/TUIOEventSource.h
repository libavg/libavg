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

#ifndef _TUIOEventSource_H_
#define _TUIOEventSource_H_

#include "../api.h"
#include "MultitouchEventSource.h"
#include "../oscpack/UdpSocket.h"
#include "../oscpack/PacketListener.h"
#include "../oscpack/OscReceivedElements.h"
#include "../oscpack/OscPrintReceivedElements.h"

#include <set>

namespace avg {

class AVG_API TUIOEventSource: public MultitouchEventSource, PacketListener
{
public:
    TUIOEventSource();
    virtual ~TUIOEventSource();
    virtual void start();
    
    virtual void ProcessPacket(const char* pData, int size, 
            const IpEndpointName& remoteEndpoint);

private:
#ifndef WIN32
    static void* threadFunc(void* p);
#else
    static DWORD WINAPI threadFunc(LPVOID* p);
#endif
    void processBundle(const osc::ReceivedBundle& bundle, 
            const IpEndpointName& remoteEndpoint);
    void processMessage(const osc::ReceivedMessage& msg, 
        const IpEndpointName& remoteEndpoint);
    void processSet(osc::ReceivedMessageArgumentStream& args);
    void processAlive(osc::ReceivedMessageArgumentStream& args);
    TouchEventPtr createEvent(int id, Event::Type type, DPoint pos, DPoint speed);

    UdpListeningReceiveSocket* m_pSocket;
    int m_LastID;
    std::set<int> m_LiveTUIOIDs;
#ifndef WIN32
    pthread_t m_Thread;
#else
    HANDLE m_Thread;
#endif  
};

typedef boost::shared_ptr<TUIOEventSource> TUIOEventSourcePtr;

}

#endif

