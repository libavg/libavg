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

#ifndef _KeyEvent_h_
#define _KeyEvent_h_

#include "../api.h"
#include "Event.h"

#include <string>

namespace avg {

class AVG_API KeyEvent : public Event {
    public:
        KeyEvent(Type eventType, int scanCode, int keyCode, 
                const std::string& keyString, int modifiers);
        virtual ~KeyEvent();

        unsigned char getScanCode() const;
        int getKeyCode() const;
        const std::string& getKeyString() const;
        int getModifiers() const;

        void trace();

    private: 
        int m_ScanCode; 
        int m_KeyCode;
        std::string m_KeyString;
        int m_Modifiers;
};

typedef boost::shared_ptr<class KeyEvent> KeyEventPtr;

}

#endif

