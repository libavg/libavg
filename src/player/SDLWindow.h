
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

#ifndef _SDLWindow_H_
#define _SDLWindow_H_

#include "../api.h"
#include "Window.h"
#include "DisplayParams.h"
#include "Event.h"

#include "../graphics/GLConfig.h"

#include <boost/shared_ptr.hpp>

union SDL_Event;

namespace avg {

class XInputMTInputDevice;
class MouseEvent;
typedef boost::shared_ptr<class MouseEvent> MouseEventPtr;
class Bitmap;
typedef boost::shared_ptr<class Bitmap> BitmapPtr;

class AVG_API SDLWindow: public Window
{
    public:
        SDLWindow(const DisplayParams& dp, GLConfig glConfig);
        ~SDLWindow();

        void setTitle(const std::string& sTitle);
        void swapBuffers() const;

        std::vector<EventPtr> pollEvents();
        void setXIMTInputDevice(XInputMTInputDevice* pInputDevice);

    private:
        void initTranslationTable();
        EventPtr createMouseEvent
                (Event::Type Type, const SDL_Event & SDLEvent, long Button);
        EventPtr createMouseButtonEvent(Event::Type Type, const SDL_Event & SDLEvent);
        EventPtr createKeyEvent(Event::Type Type, const SDL_Event & SDLEvent);
        
        // Event handling.
        MouseEventPtr m_pLastMouseEvent;
        XInputMTInputDevice * m_pXIMTInputDevice;
        static std::vector<long> s_KeyCodeTranslationTable;
};

typedef boost::shared_ptr<SDLWindow> SDLWindowPtr;

}

#endif
