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

#ifndef _TestHelper_H_
#define _TestHelper_H_

#include "../api.h"
#include "../graphics/Bitmap.h"
#include "Event.h"
#include "IInputDevice.h"

#include <boost/shared_ptr.hpp>

#include <vector>
#include <map>

namespace avg {
    
class Contact;
typedef boost::shared_ptr<class Contact> ContactPtr;
class CursorEvent;
typedef boost::shared_ptr<class CursorEvent> CursorEventPtr;

class AVG_API TestHelper : public IInputDevice
{

    public: 
        TestHelper();
        virtual ~TestHelper();
        void reset();

        void fakeMouseEvent(Event::Type eventType,
                bool leftButtonState, bool middleButtonState, 
                bool rightButtonState,
                int xPosition, int yPosition, int button);
        void fakeTouchEvent(int id, Event::Type eventType,
                Event::Source source, const DPoint& pos);
        void fakeKeyEvent(Event::Type eventType,
                unsigned char scanCode, int keyCode, 
                const std::string& keyString, int unicode, int modifiers);
        void dumpObjects();

        // From IInputDevice
        virtual std::vector<EventPtr> pollEvents();

    private:
        void insertCursorEvent(CursorEventPtr pEvent);
        void checkEventType(Event::Type eventType);
        
        std::vector<EventPtr> m_Events;
        std::map<int, ContactPtr> m_Contacts;
};

typedef boost::shared_ptr<TestHelper> TestHelperPtr;
}

#endif

