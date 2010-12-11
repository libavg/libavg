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

#include "LinuxKernelMTEventSource.h"

#include "LinuxMTHelper.h"
#include "TouchEvent.h"
#include "Player.h"
#include "AVGNode.h"
#include "TouchStatus.h"

#include "../base/Logger.h"
#include "../base/Point.h"
#include "../base/ObjectCounter.h"
#include "../base/Exception.h"

#include <linux/input.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <set>

using namespace std;

namespace avg {

LinuxKernelMTEventSource::LinuxKernelMTEventSource()
    : m_LastID(0)
{
}

LinuxKernelMTEventSource::~LinuxKernelMTEventSource()
{
}

void LinuxKernelMTEventSource::start()
{ 
    string sDevice = "/dev/input/event3";
    m_DeviceFD = ::open(sDevice.c_str(), O_RDONLY | O_NONBLOCK);
    if (m_DeviceFD == -1) {
        throw Exception(AVG_ERR_MT_INIT, 
                string("Linux multitouch event source: Could not open device file '")+
                sDevice+"'. "+strerror(errno)+".");
    }
    
    MultitouchEventSource::start();
    AVG_TRACE(Logger::CONFIG, "Linux MTDev Multitouch event source created.");
}

std::vector<EventPtr> LinuxKernelMTEventSource::pollEvents()
{
    static int numContacts = 0;
    static int tmpID;
    static IntPoint tmpPos;
    
    struct input_event event[64];
    char * pReadPos = (char*)(void*)&event;
    ssize_t numBytes = 0;
    ssize_t newBytes = -1;
    while (newBytes != 0) {
        newBytes = read(m_DeviceFD, pReadPos+numBytes, sizeof(input_event));
        if (newBytes == -1) {
            newBytes = 0;
        }
        numBytes += newBytes;
    }
    AVG_ASSERT(numBytes%sizeof(input_event) == 0);

    if (numBytes > 0) {
        cerr << "---- read ----" << endl;
    }
    for (unsigned i = 0; i < numBytes/sizeof(input_event); ++i) {
        switch (event[i].type) {
            case EV_ABS:
                cerr << "EV_ABS " << mtCodeToString(event[i].code) << endl;
                switch (event[i].code) {
                    case ABS_MT_TRACKING_ID:
                        numContacts++;
                        tmpID = event[i].value;
                        break;
                    case ABS_MT_POSITION_X:
                        tmpPos.x = event[i].value;
                        break;
                    case ABS_MT_POSITION_Y:
                        tmpPos.y = event[i].value;
                        break;
                    default:
                        break;
                }
                break;
            case EV_SYN:
                cerr << "EV_SYN " << synCodeToString(event[i].code) << endl;
                switch (event[i].code) {
                    case SYN_MT_REPORT:
                        {
                            cerr << "id: " << tmpID << ", pos: " << tmpPos << endl;
                            TouchStatusPtr pTouchStatus = getTouchStatus(tmpID);
                            if (!pTouchStatus) {
                                // Down
                                m_LastID++;
                                TouchEventPtr pEvent = createEvent(m_LastID, 
                                        Event::CURSORDOWN, tmpPos); 
                                addTouchStatus((long)tmpID, pEvent);
                            } else {
                                // Move
                                TouchEventPtr pEvent = createEvent(0, Event::CURSORMOTION,
                                        tmpPos); 
                                pTouchStatus->updateEvent(pEvent);
                            }
                        }
                        break;
                    case SYN_REPORT:
                        cerr << "SYN_REPORT" << endl;
                        break;
                    default:
                        break;
                }
                break;
            default:
                cerr << "Unexpected type " << eventTypeToString(event[i].type) << endl;
                break;
        }
    }
    return MultitouchEventSource::pollEvents();
}

TouchEventPtr LinuxKernelMTEventSource::createEvent(int id, Event::Type type, 
        IntPoint pos)
{
    DPoint size = getWindowSize();
    DPoint normPos = DPoint(double(pos.x-m_Dimensions.tl.x)/m_Dimensions.width(),
            double(pos.y-m_Dimensions.tl.y)/m_Dimensions.height());
    IntPoint screenPos(int(normPos.x*size.x+0.5), int(normPos.y*size.y+0.5));
    return TouchEventPtr(new TouchEvent(id, type, screenPos, Event::TOUCH, DPoint(0,0), 
            0, 20, 1, DPoint(5,0), DPoint(0,5)));
}
       
}
