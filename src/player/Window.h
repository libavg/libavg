
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

#ifndef _Window_H_
#define _Window_H_

#include "../api.h"
#include "WindowParams.h"
#include "DisplayParams.h"
#include "Event.h"
#include "MouseEvent.h"
#include "KeyEvent.h"

#include "../graphics/GLConfig.h"
#include "../base/Rect.h"

#include <SDL2/SDL.h>
#include <boost/shared_ptr.hpp>
#include <string>

namespace avg {

class XInputMTInputDevice;
class Bitmap;
typedef boost::shared_ptr<class Bitmap> BitmapPtr;
class GLContext;

class AVG_API Window
{
    public:
        Window(const DisplayParams& dp, const WindowParams& wp, GLConfig glConfig);
        virtual ~Window();

        void setTitle(const std::string& sTitle);
        void swapBuffers() const;
        BitmapPtr screenshot(int buffer=0);

        const IntPoint& getPos() const;
        const IntPoint& getSize() const;
        const IntRect& getViewport() const;
        bool isFullscreen() const;
        GLContext* getGLContext() const;
        std::vector<EventPtr> pollEvents();
        void setXIMTInputDevice(XInputMTInputDevice* pInputDevice);
        bool setGamma(float red, float green, float blue);

    private:
        EventPtr createMouseEvent
                (Event::Type Type, const SDL_Event & SDLEvent, long Button);
        EventPtr createMouseButtonEvent(Event::Type Type, const SDL_Event & SDLEvent);
        EventPtr createKeyEvent(Event::Type Type, const SDL_Event & SDLEvent);

        SDL_Window* m_pSDLWindow;
        SDL_GLContext m_SDLGLContext;

        // Event handling.
        glm::vec2 m_LastMousePos;
        XInputMTInputDevice * m_pXIMTInputDevice;

        bool m_bIsFullscreen;
        IntPoint m_Pos;
        IntPoint m_Size;
        IntRect m_Viewport;

        GLContext* m_pGLContext;
};

typedef boost::shared_ptr<Window> WindowPtr;

}

#endif
