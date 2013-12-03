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
    : GLXContext(glConfig, windowSize)
{
    try {
        createGLXContext(glConfig, windowSize, pSDLWMInfo, true);
    } catch (const Exception &e) {
        if (e.getCode() == AVG_ERR_DEBUG_CONTEXT_FAILED) {
            createGLXContext(glConfig, windowSize, pSDLWMInfo, false);
        } else {
            AVG_ASSERT_MSG(false, "Failed to create GLX context");
        }
    }
    init(true);
}

SDLGLXContext::~SDLGLXContext()
{
}

void SDLGLXContext::createGLXContext(const GLConfig& glConfig, const IntPoint& windowSize, 
        const SDL_SysWMinfo* pSDLWMInfo, bool bUseDebugBit)
{
    Window win = 0;
    setX11ErrorHandler();
    m_pDisplay = getX11Display(pSDLWMInfo);
    GLXFBConfig fbConfig = getFBConfig(m_pDisplay, glConfig.m_MultiSampleSamples);
    XVisualInfo* pVisualInfo = glXGetVisualFromFBConfig(m_pDisplay, fbConfig);

    if (pSDLWMInfo) {
        win = createChildWindow(pSDLWMInfo, pVisualInfo, windowSize, m_Colormap);
    }

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
    if (pSDLWMInfo) {
        setCurrent();
        glXMakeCurrent(m_pDisplay, win, m_Context);
    } else { 
        Pixmap pmp = XCreatePixmap(m_pDisplay, 
                RootWindow(m_pDisplay, pVisualInfo->screen), 8, 8, pVisualInfo->depth);
        GLXPixmap pixmap = glXCreateGLXPixmap(m_pDisplay, pVisualInfo, pmp);

        glXMakeCurrent(m_pDisplay, pixmap, m_Context);
    }
    resetX11ErrorHandler();

    throwOnXError();
    m_Drawable = glXGetCurrentDrawable();
}

}
