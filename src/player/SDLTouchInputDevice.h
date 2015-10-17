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

#ifndef _SDLTouchInputDevice_H_
#define _SDLTouchInputDevice_H_

#include "../api.h"
#include "MultitouchInputDevice.h"
#include "SDLWindow.h"

#include <SDL2/SDL.h>

namespace avg {

class SDLWindow;

class AVG_API SDLTouchInputDevice: public MultitouchInputDevice
{
public:
    SDLTouchInputDevice(const DivNodePtr& pEventReceiverNode=DivNodePtr());

    void onTouchEvent(SDLWindow* pWindow, const SDL_Event& sdlEvent);

private:
#ifdef _WIN32
    static SDLTouchInputDevice* s_pInstance;
    WNDPROC m_OldWndProc;

    static LRESULT APIENTRY touchWndSubclassProc(HWND hwnd, UINT uMsg, WPARAM wParam,
            LPARAM lParam);
#endif
};

typedef boost::shared_ptr<SDLTouchInputDevice> SDLTouchInputDevicePtr;

}

#endif
