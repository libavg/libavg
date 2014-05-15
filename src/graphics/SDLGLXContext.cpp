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

#include "SDLGLXContext.h"
#include "GLContextAttribs.h"
#include "X11Display.h"

#include "../base/Exception.h"
#include "../base/Logger.h"

#include <SDL/SDL.h>
#include <SDL/SDL_syswm.h>

#include <X11/extensions/xf86vmode.h>

#include <iostream>
#include <stdlib.h>


namespace avg {

using namespace std;
using namespace boost;

SDLGLXContext::SDLGLXContext(const GLConfig& glConfig, const IntPoint& windowSize, 
        const SDL_SysWMinfo* pSDLWMInfo)
    : GLXContext(windowSize)
{
    GLConfig config = glConfig;
    createGLXContext(config, windowSize, pSDLWMInfo);
    init(config, true);
}

SDLGLXContext::~SDLGLXContext()
{
}

void SDLGLXContext::createGLXContext(GLConfig& glConfig, const IntPoint& windowSize,
        const SDL_SysWMinfo* pSDLWMInfo)
{
    Window win = 0;
    setX11ErrorHandler();
    XVisualInfo* pVisualInfo = createDetachedContext(getX11Display(pSDLWMInfo), glConfig);

    if (pSDLWMInfo) {
        win = createChildWindow(pSDLWMInfo, pVisualInfo, windowSize, getColormap());
        setCurrent();
        glXMakeCurrent(getDisplay(), win, getGLXContext());
    } else { 
        Pixmap pmp = XCreatePixmap(getDisplay(), 
                RootWindow(getDisplay(), pVisualInfo->screen), 8, 8, pVisualInfo->depth);
        GLXPixmap pixmap = glXCreateGLXPixmap(getDisplay(), pVisualInfo, pmp);

        glXMakeCurrent(getDisplay(), pixmap, getGLXContext());
    }
    resetX11ErrorHandler();

    throwOnXError();
    initDrawable();
}

}
