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

#include "LibMTDevInputDevice.h"

#include "TouchEvent.h"
#include "Player.h"
#include "AVGNode.h"
#include "TouchStatus.h"

#include "../base/Logger.h"
#include "../base/Point.h"
#include "../base/ObjectCounter.h"
#include "../base/Exception.h"
#include "../base/OSHelper.h"

#include <linux/input.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <set>

extern "C" {
#include <mtdev.h>
#include <mtdev-mapping.h>
}

using namespace std;

namespace avg {

LibMTDevInputDevice::LibMTDevInputDevice()
    : m_LastID(0),
      m_pMTDevice(0)
{
}

LibMTDevInputDevice::~LibMTDevInputDevice()
{
    if (m_pMTDevice) {
        mtdev_close(m_pMTDevice);
        delete m_pMTDevice;
    }
}

void LibMTDevInputDevice::start()
{ 
    string sDevice("/dev/input/event3");
    getEnv("AVG_LINUX_MULTITOUCH_DEVICE", sDevice);
    m_DeviceFD = ::open(sDevice.c_str(), O_RDONLY | O_NONBLOCK);
    if (m_DeviceFD == -1) {
        throw Exception(AVG_ERR_MT_INIT, 
                string("Linux multitouch event source: Could not open device file '")+
                sDevice+"'. "+strerror(errno)+".");
    }
    m_pMTDevice = new mtdev;
    int err = mtdev_open(m_pMTDevice, m_DeviceFD);
    if (err == -1) {
        throw Exception(AVG_ERR_MT_INIT, 
                string("Linux multitouch event source: Could not open mtdev '")+
                sDevice+"'. "+strerror(errno)+".");
    }
    input_absinfo* pAbsInfo;
    pAbsInfo = &(m_pMTDevice->caps.abs[MTDEV_POSITION_X]);
    m_Dimensions.tl.x = pAbsInfo->minimum;
    m_Dimensions.br.x = pAbsInfo->maximum;
    pAbsInfo = &(m_pMTDevice->caps.abs[MTDEV_POSITION_Y]);
    m_Dimensions.tl.y = pAbsInfo->minimum;
    m_Dimensions.br.y = pAbsInfo->maximum;
    
    MultitouchInputDevice::start();
    AVG_TRACE(Logger::CONFIG, "Linux MTDev Multitouch event source created.");
}

std::vector<EventPtr> LibMTDevInputDevice::pollEvents()
{
    struct input_event events[64];
    int numEvents = mtdev_get(m_pMTDevice, m_DeviceFD, events, 64);
/*
    if (numEvents > 0) {
        cerr << "---- read ----" << endl;
    }
*/
    static int curSlot = 0;

    set<int> changedIDs;
    for (int i = 0; i < numEvents; ++i) {
        input_event event = events[i];
        if (event.type == EV_ABS && event.code == ABS_MT_SLOT) {
//            cerr << ">> slot " << event.value << endl;
            curSlot = event.value;
        } else {
            TouchData* pTouch;
            switch (event.code) {
                case ABS_MT_TRACKING_ID:
//                    cerr << ">> ABS_MT_TRACKING_ID: " << event.value << endl;
                    pTouch = &(m_Slots[curSlot]);
                    if (event.value == -1) {
                        TouchStatusPtr pTouchStatus = getTouchStatus(pTouch->id);
//                        cerr << "up " << pTouch->id << endl;
                        if (pTouchStatus) {
//                            cerr << "  --> remove" << endl;
                            TouchEventPtr pOldEvent = pTouchStatus->getLastEvent();
                            TouchEventPtr pUpEvent = 
                                    boost::dynamic_pointer_cast<TouchEvent>(
                                    pOldEvent->cloneAs(Event::CURSORUP));
                            pTouchStatus->updateEvent(pUpEvent);
                        }
                        pTouch->id = -1;
                    } else {
                        pTouch->id = event.value;
                    }
                    break;
                case ABS_MT_POSITION_X:
//                    cerr << ">> ABS_MT_POSITION_X: " << event.value << endl;
                    pTouch = &(m_Slots[curSlot]);
                    pTouch->pos.x = event.value;
                    break;
                case ABS_MT_POSITION_Y:
//                    cerr << ">> ABS_MT_POSITION_Y: " << event.value << endl;
                    pTouch = &(m_Slots[curSlot]);
                    pTouch->pos.y = event.value;
                    break;
                default:
                    break;
            }
//            cerr << ">> new id: " << curSlot << endl;
            changedIDs.insert(curSlot);
        }
    }
    for (set<int>::iterator it = changedIDs.begin(); it != changedIDs.end(); ++it) {
        map<int, TouchData>::iterator it2 = m_Slots.find(*it);
        if (it2 != m_Slots.end()) {
            const TouchData& touch = it2->second;
//            cerr << "slot: " << *it << ", id: " << touch.id << ", pos: " << touch.pos 
//                    << endl;
//            AVG_ASSERT(touch.pos.x != 0);
            if (touch.id != -1) {
                TouchStatusPtr pTouchStatus = getTouchStatus(touch.id);
                if (!pTouchStatus) {
//                    cerr << "down" << endl;
                    // Down
                    m_LastID++;
                    TouchEventPtr pEvent = createEvent(m_LastID, Event::CURSORDOWN, 
                            touch.pos); 
                    addTouchStatus((long)touch.id, pEvent);
                } else {
//                    cerr << "move" << endl;
                    // Move
                    TouchEventPtr pEvent = createEvent(0, Event::CURSORMOTION, touch.pos); 
                    pTouchStatus->updateEvent(pEvent);
                }
            }
        }
    }
    return MultitouchInputDevice::pollEvents();
}

TouchEventPtr LibMTDevInputDevice::createEvent(int id, Event::Type type, IntPoint pos)
{
    DPoint size = getWindowSize();
    DPoint normPos = DPoint(double(pos.x-m_Dimensions.tl.x)/m_Dimensions.width(),
            double(pos.y-m_Dimensions.tl.y)/m_Dimensions.height());
    IntPoint screenPos(int(normPos.x*size.x+0.5), int(normPos.y*size.y+0.5));
    return TouchEventPtr(new TouchEvent(id, type, screenPos, Event::TOUCH, DPoint(0,0), 
            0, 20, 1, DPoint(5,0), DPoint(0,5)));
}
           
}
