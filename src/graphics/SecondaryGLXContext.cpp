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

void SecondaryGLXContext::createContext(const GLConfig& glConfig, const string& sDisplay, 
        const IntRect& windowDimensions, bool bUseDebugBit)
{
    setX11ErrorHandler();

    m_pDisplay = XOpenDisplay(sDisplay.c_str());
    GLXFBConfig fbConfig = getFBConfig(m_pDisplay, glConfig.m_MultiSampleSamples);
    XVisualInfo* pVisualInfo = glXGetVisualFromFBConfig(m_pDisplay, fbConfig);

    ::Window rootWindow = DefaultRootWindow(m_pDisplay);
    m_Colormap = XCreateColormap(m_pDisplay, rootWindow, pVisualInfo->visual, 
            AllocNone);
    AVG_ASSERT(m_Colormap);
    XSetWindowAttributes swa;
    swa.event_mask = ButtonPressMask;
    swa.colormap = m_Colormap;

    m_Window = XCreateWindow(m_pDisplay, rootWindow, 0, 0, 800, 600, 5, pVisualInfo->depth, 
            InputOutput, pVisualInfo->visual, CWColormap | CWEventMask, &swa);
    AVG_ASSERT(m_Window);
    XMapWindow(m_pDisplay, m_Window);
    XStoreName(m_pDisplay, m_Window, "libavg secondary window");
    
    if (haveARBCreateContext()) {
        GLContextAttribs attrs;
        if (isGLES()) {
            attrs.append(GLX_CONTEXT_PROFILE_MASK_ARB, GLX_CONTEXT_ES2_PROFILE_BIT_EXT);
            attrs.append(GLX_CONTEXT_MAJOR_VERSION_ARB, 2);
            attrs.append(GLX_CONTEXT_MINOR_VERSION_ARB, 0);
        }
        if (glConfig.m_bUseDebugContext && bUseDebugBit) {
            attrs.append(GLX_CONTEXT_FLAGS_ARB, GLX_CONTEXT_DEBUG_BIT_ARB);
        }
        PFNGLXCREATECONTEXTATTRIBSARBPROC CreateContextAttribsARB = 
            (PFNGLXCREATECONTEXTATTRIBSARBPROC)
            getglXProcAddress("glXCreateContextAttribsARB");

//        s_bDumpX11ErrorMsg = false;
        m_Context = CreateContextAttribsARB(m_pDisplay, fbConfig, 0, 1, attrs.get());
//        s_bDumpX11ErrorMsg = true;
        throwOnXError(AVG_ERR_DEBUG_CONTEXT_FAILED);
    } else {
        m_Context = glXCreateContext(m_pDisplay, pVisualInfo, 0, GL_TRUE);
    }
    AVG_ASSERT(m_Context);
    setCurrent();
    glXMakeCurrent(m_pDisplay, m_Window, m_Context);
    
    resetX11ErrorHandler();

    throwOnXError();
    m_Drawable = glXGetCurrentDrawable();
}

SecondaryGLXContext::~SecondaryGLXContext()
{
}

}
