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

#include "SecondaryGLXContext.h"
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

SecondaryGLXContext::SecondaryGLXContext(const GLConfig& glConfig, const string& sDisplay,
        const IntRect& windowDimensions)
    : GLXContext(glConfig, windowDimensions.size())
{
    try {
        createContext(glConfig, sDisplay, windowDimensions, true);
    } catch (const Exception &e) {
        if (e.getCode() == AVG_ERR_DEBUG_CONTEXT_FAILED) {
            createContext(glConfig, sDisplay, windowDimensions, false);
        } else {
            AVG_ASSERT_MSG(false, "Failed to create GLX context");
        }
    }
    init(true);
}

SecondaryGLXContext::~SecondaryGLXContext()
{
    XDestroyWindow(getDisplay(), m_Window);
}

void SecondaryGLXContext::createContext(const GLConfig& glConfig, const string& sDisplay, 
        const IntRect& windowDimensions, bool bUseDebugBit)
{
    setX11ErrorHandler();

    XVisualInfo* pVisualInfo = createDetachedContext(XOpenDisplay(sDisplay.c_str()), 
            glConfig, bUseDebugBit);
    
    XSetWindowAttributes swa;
    swa.event_mask = ButtonPressMask;
    swa.colormap = getColormap();

    m_Window = XCreateWindow(getDisplay(), DefaultRootWindow(getDisplay()), 
            windowDimensions.tl.x, windowDimensions.tl.y, windowDimensions.width(), 
            windowDimensions.height(), 5, pVisualInfo->depth, InputOutput, 
            pVisualInfo->visual, CWColormap | CWEventMask, &swa);
    AVG_ASSERT(m_Window);
    XMapWindow(getDisplay(), m_Window);
    XStoreName(getDisplay(), m_Window, "libavg secondary window");
    
    setCurrent();
    glXMakeCurrent(getDisplay(), m_Window, getGLXContext());
    
    resetX11ErrorHandler();

    throwOnXError();
    initDrawable();
}

}
