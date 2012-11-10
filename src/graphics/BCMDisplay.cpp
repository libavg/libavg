//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2012 Ulrich von Zadow
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

#include "BCMDisplay.h"

#include "../base/Exception.h"

#include <SDL/SDL.h>
#include <SDL/SDL_syswm.h>

namespace avg {

DISPMANX_DISPLAY_HANDLE_T getBCMDisplay(const SDL_SysWMinfo* pSDLWMInfo)
{
    bcm_host_init();

    DISPMANX_DISPLAY_HANDLE_T display = vc_dispmanx_display_open(DISPMANX_ID_MAIN_LCD);
    if (display == DISPMANX_NO_HANDLE) {
        throw Exception(AVG_ERR_VIDEO_GENERAL, "No BCM display available.");
    }
    return display;
}

EGL_DISPMANX_WINDOW_T* createChildWindow(const SDL_SysWMinfo* pSDLWMInfo,
        const IntPoint& windowSize)
{
    // Create a child window with the required attributes to render into.
    EGL_DISPMANX_WINDOW_T* pWin = new EGL_DISPMANX_WINDOW_T; // XXX: destroy
    pWin->element = 0; // XXX: create BCM display element
    pWin->width = windowSize.x;
    pWin->height = windowSize.y;
    return pWin;
}

}
