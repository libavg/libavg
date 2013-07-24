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

#include "MultitouchInputDevice.h"
#include "TouchEvent.h"
#include "Player.h"
#include "AVGNode.h"
#include "TouchStatus.h"

#include "../base/Logger.h"
#include "../base/ObjectCounter.h"
#include "../base/Exception.h"

using namespace std;

namespace avg {

MultitouchInputDevice::MultitouchInputDevice()
    : IInputDevice(EXTRACT_INPUTDEVICE_CLASSNAME(MultitouchInputDevice))
{
    mTouchArea = ConfigMgr::get()->getSizeOption("touch", "area");
    if(mTouchArea.x == 0){
        mTouchArea = Player::get()->getScreenResolution();
    }
    mTouchOffset = ConfigMgr::get()->getSizeOption("touch", "offset");
}

MultitouchInputDevice::~MultitouchInputDevice()
{
}

void MultitouchInputDevice::start()
{
    m_pMutex = MutexPtr(new boost::mutex);
}

vector<EventPtr> MultitouchInputDevice::pollEvents()
{
    boost::mutex::scoped_lock lock(*m_pMutex);

    vector<EventPtr> events;
    vector<TouchStatusPtr>::iterator it;
//    cerr << "--------poll---------" << endl;
    for (it = m_Touches.begin(); it != m_Touches.end(); ) {
//        cerr << it->first << " ";
        CursorEventPtr pEvent = (*it)->pollEvent();
        if (pEvent) {
            events.push_back(pEvent);
            if (pEvent->getType() == Event::CURSOR_UP) {
                it = m_Touches.erase(it);
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

int MultitouchInputDevice::getNumTouches() const
{
    return m_TouchIDMap.size();
}

TouchStatusPtr MultitouchInputDevice::getTouchStatus(int id)
{
    map<int, TouchStatusPtr>::iterator it = m_TouchIDMap.find(id);
    if (it == m_TouchIDMap.end()) {
        return TouchStatusPtr();
    } else {
        return it->second;
    }
}

void MultitouchInputDevice::addTouchStatus(int id, TouchEventPtr pInitialEvent)
{
    TouchStatusPtr pTouchStatus(new TouchStatus(pInitialEvent));
    m_TouchIDMap[id] = pTouchStatus;
    m_Touches.push_back(pTouchStatus);
}

void MultitouchInputDevice::removeTouchStatus(int id)
{
    unsigned numRemoved = m_TouchIDMap.erase(id);
    AVG_ASSERT(numRemoved == 1);
}

void MultitouchInputDevice::getDeadIDs(const set<int>& liveIDs, set<int>& deadIDs)
{
    map<int, TouchStatusPtr>::iterator it;
    for (it = m_TouchIDMap.begin(); it != m_TouchIDMap.end(); ++it) {
        int id = it->first;
        set<int>::const_iterator foundIt = liveIDs.find(id);
        if (foundIt == liveIDs.end()) {
            deadIDs.insert(id);
        }
    }
}

glm::vec2 MultitouchInputDevice::getTouchArea() const
{
    return mTouchArea;
}

IntPoint MultitouchInputDevice::getScreenPos(const glm::vec2& pos) const
{
        return IntPoint(int(pos.x * mTouchArea.x + mTouchOffset.x),
                        int(pos.y * mTouchArea.y + mTouchOffset.y));
}

boost::mutex& MultitouchInputDevice::getMutex()
{
    return *m_pMutex;
}

}
