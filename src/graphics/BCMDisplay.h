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
#ifndef _BCMDisplay_H_
#define _BCMDisplay_H_

#include <bcm_host.h>

#include "../api.h"

#include "../base/GLMHelper.h"

#include "OGLHelper.h"

struct SDL_SysWMinfo;

namespace avg {

DISPMANX_DISPLAY_HANDLE_T getBCMDisplay(const SDL_SysWMinfo* pSDLWMInfo);

EGL_DISPMANX_WINDOW_T* createChildWindow(const SDL_SysWMinfo* pSDLWMInfo,
        EGLNativeDisplayType bcmDisplay, const IntPoint& windowSize);

EGLSurface createBCMPixmapSurface(EGLDisplay display, EGLConfig config);

}

#endif
