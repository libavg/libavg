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
//  Original author of this file is igor@c-base.org
//

#include "TUIOEventSource.h"
#include "TouchEvent.h"
#include "Player.h"
#include "AVGNode.h"
#include "TouchStatus.h"

#include "../base/Logger.h"
#include "../base/ObjectCounter.h"
#include "../base/Exception.h"

using namespace std;
using namespace osc;

namespace avg {

TUIOEventSource::TUIOEventSource()
    : m_LastID(0)
{
}

TUIOEventSource::~TUIOEventSource()
{
}

void TUIOEventSource::start()
{
    MultitouchEventSource::start();
    try {
        m_pSocket = new UdpListeningReceiveSocket(IpEndpointName(
                IpEndpointName::ANY_ADDRESS, 3333), this);
    } catch (std::exception &e) {
        cerr << e.what() << endl;
        throw;
    }
    if (!m_pSocket->IsBound()) {
        cerr << "socket not bound" << endl;
    }
    cerr << "TUIOEventSource created" << endl;
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
    cerr << *pData << endl;
    try {
        ReceivedPacket packet(pData, size);
        if (packet.IsBundle()) {
            processBundle(ReceivedBundle(packet), remoteEndpoint);
        } else {
            processMessage(ReceivedMessage(packet), remoteEndpoint);
        }
    } catch (MalformedBundleException& e) {
        std::cerr << "malformed OSC bundle: " << e.what() << std::endl;
    }
}

void TUIOEventSource::processBundle(const ReceivedBundle& bundle, 
        const IpEndpointName& remoteEndpoint) 
{
    for (ReceivedBundle::const_iterator it = bundle.ElementsBegin(); 
            it != bundle.ElementsEnd(); ++it) 
    {
        if (it->IsBundle()) {
            processBundle(ReceivedBundle(*it), remoteEndpoint);
        } else {
            processMessage(ReceivedMessage(*it), remoteEndpoint);
        }
    }
}

void TUIOEventSource::processMessage(const ReceivedMessage& msg, 
        const IpEndpointName& remoteEndpoint) 
{
    cerr << msg << endl;
}

#ifndef WIN32
void* TUIOEventSource::threadFunc(void* p)
#else
DWORD WINAPI TUIOEventSource::threadFunc(LPVOID* p)
#endif
{
    TUIOEventSource* pThis = (TUIOEventSource*)p;
    pThis->m_pSocket->Run();
    return 0;
};

}
