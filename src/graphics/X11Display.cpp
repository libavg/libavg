//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2020 Ulrich von Zadow
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
#include "../base/Logger.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>

#include <boost/math/special_functions/fpclassify.hpp>


namespace avg {

X11Display::X11Display()
{
}

X11Display::~X11Display()
{
}
 
float X11Display::queryPPMM()
{
    ::Display * pDisplay = getX11Display();
    float ppmm = getScreenResolution().x/float(DisplayWidthMM(pDisplay, 0));
    XCloseDisplay(pDisplay);
    return ppmm;
}

::Display* getX11Display()
{
    ::Display* pDisplay;
    pDisplay = XOpenDisplay(0);
    if (!pDisplay) {
        throw Exception(AVG_ERR_VIDEO_GENERAL, "Could not open X11 display.");
    }
    return pDisplay;
}

}
