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
#ifndef _X11Display_H_
#define _X11Display_H_
#include "../api.h"

#include "Display.h"

#include "../base/GLMHelper.h"

#include "OGLHelper.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/xf86vmode.h>

struct SDL_SysWMinfo;

namespace avg {

class AVG_API X11Display: public Display
{
public:
    X11Display();
    virtual ~X11Display();
 
protected:
    virtual float queryPPMM();
    virtual IntPoint queryScreenResolution();
    virtual float queryRefreshRate();
};

::Display* getX11Display(const SDL_SysWMinfo* pSDLWMInfo);

Window createChildWindow(const SDL_SysWMinfo* pSDLWMInfo, XVisualInfo* pVisualInfo,
        const IntPoint& windowSize, const Colormap& colormap);

}
#endif


