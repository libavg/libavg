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

#include "X11Display.h"

#include "../base/Exception.h"

#include <SDL/SDL.h>
#include <SDL/SDL_syswm.h>

#include <X11/extensions/xf86vmode.h>

namespace avg {

Display* getX11Display(const SDL_SysWMinfo* pSDLWMInfo)
{
    Display* pDisplay;
    if (pSDLWMInfo) {
        // SDL window exists, use it.
        pDisplay = pSDLWMInfo->info.x11.display;
    } else {
        pDisplay = XOpenDisplay(0);
    }
    if (!pDisplay) {
        throw Exception(AVG_ERR_VIDEO_GENERAL, "No X windows display available.");
    }
    return pDisplay;
}

Window createChildWindow(const SDL_SysWMinfo* pSDLWMInfo, XVisualInfo* pVisualInfo,
        const IntPoint& windowSize, Colormap& colormap)

{
    // Create a child window with the required attributes to render into.
    XSetWindowAttributes swa;
    Display* pDisplay = pSDLWMInfo->info.x11.display;
    colormap = XCreateColormap(pDisplay, RootWindow(pDisplay, pVisualInfo->screen),
            pVisualInfo->visual, AllocNone);
    swa.colormap = colormap;
    swa.background_pixmap = None;
    swa.event_mask = StructureNotifyMask; 
    Window win = XCreateWindow(pDisplay, pSDLWMInfo->info.x11.window, 
            0, 0, windowSize.x, windowSize.y, 0, pVisualInfo->depth, InputOutput, 
            pVisualInfo->visual, CWColormap|CWEventMask, &swa);
    AVG_ASSERT(win);
    XMapWindow(pDisplay, win);
    return win;
}

}
