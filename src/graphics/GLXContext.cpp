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

#include "GLXContext.h"
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

static bool s_bX11Error;
static bool s_bDumpX11ErrorMsg;
static int (*s_DefaultErrorHandler) (::Display *, XErrorEvent *);

GLXContext::GLXContext(const GLConfig& glConfig, const IntPoint& windowSize)
    : GLContext(glConfig, windowSize),
      m_pDisplay(0)
{
    s_bX11Error = false;
}

GLXContext::~GLXContext()
{
    deleteObjects();
    if (m_Context && ownsContext()) {
        glXMakeCurrent(m_pDisplay, 0, 0);
        glXDestroyContext(m_pDisplay, m_Context);
        m_Context = 0;
        XDestroyWindow(m_pDisplay, m_Drawable);
        XFreeColormap(m_pDisplay, m_Colormap);
    }
}

void GLXContext::activate()
{
    glXMakeCurrent(m_pDisplay, m_Drawable, m_Context);
    setCurrent();
}

bool GLXContext::initVBlank(int rate) 
{
    static bool s_bVBlankActive = false;
    if (rate > 0) {
        if (getenv("__GL_SYNC_TO_VBLANK") != 0) {
            AVG_LOG_WARNING("__GL_SYNC_TO_VBLANK set. This interferes with libavg vblank handling.");
            s_bVBlankActive = false;
            return false;
        } 
        if (!queryGLXExtension("GLX_EXT_swap_control")) {
            AVG_LOG_WARNING("Linux VBlank setup failed: OpenGL Extension not supported.");
            s_bVBlankActive = false;
            return false;
        }

        glproc::SwapIntervalEXT(m_pDisplay, m_Drawable, rate);
        s_bVBlankActive = true;
        return true;

    } else {
        if (s_bVBlankActive) {
            glproc::SwapIntervalEXT(m_pDisplay, m_Drawable, 0);
            s_bVBlankActive = false;
        }
        return false;
    }
}

bool GLXContext::useDepthBuffer() const
{
    // NVidia GLX GLES doesn't allow framebuffer stencil without depth.
    return true;
}

void GLXContext::swapBuffers()
{
    glXSwapBuffers(m_pDisplay, m_Drawable);
}

bool GLXContext::haveARBCreateContext()
{
    static bool s_bExtensionChecked = false;
    static bool s_bHaveExtension = false;
    if (!s_bExtensionChecked) {
        s_bExtensionChecked = true;
        s_bHaveExtension = (queryGLXExtension("GLX_ARB_create_context"));
    }
    return s_bHaveExtension;
}

XVisualInfo* GLXContext::createDetachedContext(::Display* pDisplay,
        const GLConfig& glConfig, bool bUseDebugBit)
{
    m_pDisplay = pDisplay;
    GLXFBConfig fbConfig = getFBConfig(m_pDisplay, glConfig.m_MultiSampleSamples);
    XVisualInfo* pVisualInfo = glXGetVisualFromFBConfig(m_pDisplay, fbConfig);

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

        s_bDumpX11ErrorMsg = false;
        m_Context = CreateContextAttribsARB(m_pDisplay, fbConfig, 0, 1, attrs.get());
        s_bDumpX11ErrorMsg = true;
        throwOnXError(AVG_ERR_DEBUG_CONTEXT_FAILED);
    } else {
        m_Context = glXCreateContext(m_pDisplay, pVisualInfo, 0, GL_TRUE);
    }
    AVG_ASSERT(m_Context);
    
    m_Colormap = XCreateColormap(pDisplay, RootWindow(m_pDisplay, pVisualInfo->screen),
            pVisualInfo->visual, AllocNone);
    AVG_ASSERT(m_Colormap);

    return pVisualInfo;
}

void GLXContext::setX11ErrorHandler()
{
    s_bDumpX11ErrorMsg = true;
    s_DefaultErrorHandler = XSetErrorHandler(X11ErrorHandler);
}

void GLXContext::resetX11ErrorHandler()
{
    XSetErrorHandler(s_DefaultErrorHandler);
}

void GLXContext::throwOnXError(int code)
{
    if (s_bX11Error) {
        throw Exception(code, "X error creating OpenGL context.");
    }
}

void GLXContext::initDrawable()
{
    m_Drawable = glXGetCurrentDrawable();
}

::GLXContext GLXContext::getGLXContext() const
{
    return m_Context;
}

::Display* GLXContext::getDisplay() const
{
    return m_pDisplay;
}

Colormap GLXContext::getColormap() const
{
    return m_Colormap;
}

GLXFBConfig GLXContext::getFBConfig(::Display* pDisplay, int multiSampleSamples)
{
    GLContextAttribs attrs;
    attrs.append(GLX_X_RENDERABLE, 1);
    attrs.append(GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT);
    attrs.append(GLX_RENDER_TYPE, GLX_RGBA_BIT);
    attrs.append(GLX_X_VISUAL_TYPE, GLX_TRUE_COLOR);
    attrs.append(GLX_DEPTH_SIZE, 0);
    attrs.append(GLX_STENCIL_SIZE, 8);
    attrs.append(GLX_DOUBLEBUFFER, 1);
    attrs.append(GLX_RED_SIZE, 8);
    attrs.append(GLX_GREEN_SIZE, 8);
    attrs.append(GLX_BLUE_SIZE, 8);
    attrs.append(GLX_ALPHA_SIZE, 0);

    int fbCount;
    GLXFBConfig* pFBConfig = glXChooseFBConfig(m_pDisplay, DefaultScreen(m_pDisplay), 
            attrs.get(), &fbCount);
    if (!pFBConfig) {
        throw Exception(AVG_ERR_UNSUPPORTED, "Creating OpenGL context failed.");
    }
    
    // Find the config with the appropriate number of multisample samples.
    int bestConfig = -1;
    int bestSamples = -1;
    for (int i=0; i<fbCount; ++i) {
        XVisualInfo* pVisualInfo = glXGetVisualFromFBConfig(m_pDisplay, pFBConfig[i]);
        if (pVisualInfo) {
            int buffer;
            int samples;
            glXGetFBConfigAttrib(m_pDisplay, pFBConfig[i], GLX_SAMPLE_BUFFERS, &buffer);
            glXGetFBConfigAttrib(m_pDisplay, pFBConfig[i], GLX_SAMPLES, &samples);
            if (bestConfig < 0 || 
                    (buffer == 1 && samples > bestSamples && 
                     samples <= multiSampleSamples))
            {
                bestConfig = i;
                bestSamples = samples;
            }
            XFree(pVisualInfo);
        }
    }
    GLXFBConfig fbConfig = pFBConfig[bestConfig];
    XFree(pFBConfig);
    return fbConfig;
}

int GLXContext::X11ErrorHandler(::Display * pDisplay, XErrorEvent * pErrEvent)
{
    if (s_bDumpX11ErrorMsg) {
        char errorString[128]; 
        XGetErrorText(pDisplay, pErrEvent->error_code, errorString, 128);
        cerr << "X11 error creating GL context: " << errorString <<
            "\n\tMajor opcode of failed request: " << (int)(pErrEvent->request_code) <<
            "\n\tMinor opcode of failed request: " << (int)(pErrEvent->minor_code) <<
            "\n";
    }
    s_bX11Error = true;
    return 0;
}

}
