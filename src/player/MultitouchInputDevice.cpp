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

#include "MultitouchInputDevice.h"
#include "TouchEvent.h"
#include "Player.h"
#include "AVGNode.h"
#include "Contact.h"

#include "../base/Logger.h"
#include "../base/ObjectCounter.h"
#include "../base/Exception.h"

using namespace std;

namespace avg {

MultitouchInputDevice::MultitouchInputDevice()
    : IInputDevice(EXTRACT_INPUTDEVICE_CLASSNAME(MultitouchInputDevice))
{
}

MultitouchInputDevice::~MultitouchInputDevice()
{
}

void MultitouchInputDevice::start()
{
    m_WindowSize = Player::get()->getRootNode()->getSize();
    m_pMutex = MutexPtr(new boost::mutex);
}

vector<EventPtr> MultitouchInputDevice::pollEvents()
{
    boost::mutex::scoped_lock lock(*m_pMutex);

    vector<EventPtr> events;
    map<int, ContactPtr>::iterator it;
//    cerr << "--------poll---------" << endl;
    for (it = m_Touches.begin(); it != m_Touches.end(); ) {
//        cerr << it->first << " ";
        ContactPtr pContact = it->second;
        CursorEventPtr pEvent = pContact->pollEvent();
        if (pEvent) {
            events.push_back(pEvent);
            if (pEvent->getType() == Event::CURSORUP) {
                m_Touches.erase(it++);
            } else {
                ++it;
            }
        } else {
            ++it;
        }
    }
//    cerr << endl;
    return events;
}

const DPoint& MultitouchInputDevice::getWindowSize() const
{
    return m_WindowSize;
}

int MultitouchInputDevice::getNumTouches() const
{
    return m_Touches.size();
}

ContactPtr MultitouchInputDevice::getContact(int id)
{
    map<int, ContactPtr>::iterator it = m_Touches.find(id);
    if (it == m_Touches.end()) {
        return ContactPtr();
    } else {
        return it->second;
    }
}

void MultitouchInputDevice::addContact(int id, TouchEventPtr pInitialEvent)
{
    ContactPtr pContact(new Contact(pInitialEvent));
    pContact->setThis(pContact);
    m_Touches[id] = pContact;
}
    
void MultitouchInputDevice::getDeadIDs(const set<int>& liveIDs, set<int>& deadIDs)
{
    map<int, ContactPtr>::iterator it;
    for (it = m_Touches.begin(); it != m_Touches.end(); ++it) {
        int id = it->first;
        set<int>::const_iterator foundIt = liveIDs.find(id);
        if (foundIt == liveIDs.end()) {
            deadIDs.insert(id);
        }
    }
}

boost::mutex& MultitouchInputDevice::getMutex()
{
    return *m_pMutex;
}

}
