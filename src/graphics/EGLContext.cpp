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

#include "EGLContext.h"
#include "GLContextAttribs.h"

#include "../base/Exception.h"
#include "../base/Logger.h"

#include <SDL/SDL.h>
#include <SDL/SDL_syswm.h>
#include <EGL/egl.h>

#include <iostream>

namespace avg{

using namespace std;

EGLContext::EGLContext(const GLConfig& glConfig, const IntPoint& windowSize,
        const SDL_SysWMinfo* pSDLWMInfo)
    : GLContext(glConfig, windowSize, pSDLWMInfo)
{
    createEGLContext(glConfig, windowSize, pSDLWMInfo);
    init(true);
}

EGLContext::~EGLContext()
{
    getVertexBufferCache().deleteBuffers();
    getIndexBufferCache().deleteBuffers();
    getPBOCache().deleteBuffers();
    eglTerminate(m_Display);
}

void EGLContext::createEGLContext(const GLConfig& glConfig, const IntPoint& windowSize,
        const SDL_SysWMinfo* pSDLWMInfo)
{
    if (pSDLWMInfo) {
        // SDL window exists, use it.
        m_xDisplay = (EGLNativeDisplayType)pSDLWMInfo->info.x11.display;
    } else {
        m_xDisplay = (EGLNativeDisplayType)XOpenDisplay(0);
    }
    if (!m_xDisplay) {
        throw Exception(AVG_ERR_VIDEO_GENERAL, "No X window display available.");
    }

    m_Display = eglGetDisplay(m_xDisplay);
    if (m_Display == EGL_NO_DISPLAY) {
        throw Exception(AVG_ERR_VIDEO_GENERAL, "No EGL display available.");
    }

    if (!eglInitialize(m_Display, NULL, NULL)) {
        throw Exception(AVG_ERR_VIDEO_GENERAL, "Unable to initialize EGL.");
    }

    GLContextAttribs fbAttrs;
    fbAttrs.append(EGL_RED_SIZE, 1);
    fbAttrs.append(EGL_GREEN_SIZE, 1);
    fbAttrs.append(EGL_BLUE_SIZE, 1);
    fbAttrs.append(EGL_DEPTH_SIZE, 1);
    fbAttrs.append(EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT);
    EGLint numFBConfig;
    if (!eglChooseConfig(m_Display, fbAttrs.get(), &m_Config, 1, &numFBConfig)) {
        cerr << "Failed to choose config (eglError: " << eglGetError() << ")" << endl;
        return;
    }
    AVG_ASSERT(m_Config);
    AVG_ASSERT(numFBConfig > 0);

    EGLint vid;
    if (!eglGetConfigAttrib(m_Display, m_Config, EGL_NATIVE_VISUAL_ID, &vid)) {
        cerr << "Error: eglGetConfigAttrib() failed\n";
        return;
    }

    XVisualInfo *visInfo, visTemplate;

    visTemplate.visualid = vid;

    int num_visuals;
    visInfo = XGetVisualInfo(m_xDisplay, VisualIDMask, &visTemplate, &num_visuals);
    if (!visInfo) {
        cerr << "Error: Could not get X Visual\n";
        return;
    }

    m_xWindow = 0;
    if (pSDLWMInfo) {
        Window root = pSDLWMInfo->info.x11.window;

        XSetWindowAttributes attr;
        attr.background_pixel = 0;
        attr.border_pixel = 0;
        attr.colormap = XCreateColormap( m_xDisplay, root, visInfo->visual, AllocNone);

        unsigned long mask = CWBackPixel | CWBorderPixel | CWColormap;
        m_xWindow = XCreateWindow(m_xDisplay, root, 
                0, 0, windowSize.x, windowSize.y, 0, visInfo->depth, InputOutput, 
                visInfo->visual, mask, &attr);
        AVG_ASSERT(m_xWindow);
        XMapWindow(m_xDisplay, m_xWindow);
    }
 
    if(!eglBindAPI(EGL_OPENGL_ES_API)) {
        cerr << "Failed to bind GLES API to EGL\n";
        return;
    }

    if (numFBConfig != 1) {
        cerr << "Didn't get exactly one config, but " << numFBConfig << endl;
        return;
    }
    if (m_xWindow) {
        m_Surface = eglCreateWindowSurface(m_Display, m_Config, m_xWindow, NULL);
    } else {
        XVisualInfo visTemplate, *results;
        visTemplate.screen = 0;
        int numVisuals;
        results = XGetVisualInfo(m_xDisplay, VisualScreenMask,
                    &visTemplate, & numVisuals);
        
        Pixmap pmp = XCreatePixmap(m_xDisplay, 
                RootWindow(m_xDisplay, results[0].screen), 8, 8, results[0].depth);
        m_Surface = eglCreatePixmapSurface(m_Display, m_Config, pmp, NULL);
    }
    if (m_Surface == EGL_NO_SURFACE) {
        cerr << "Unable to create EGL surface (eglError: " << eglGetError() << ")" << endl;
        return;
    }
    GLContextAttribs attrs;
    attrs.append(EGL_CONTEXT_CLIENT_VERSION, 2);

    m_Context = eglCreateContext(m_Display, m_Config, NULL, attrs.get());
    if (m_Context == 0) {
        cerr << "Unable to create EGL context (eglError: " << eglGetError() << ")" << endl;
        return;
    }
}

bool EGLContext::initVBlank(int rate)
{
    return false;
}

void EGLContext::activate()
{
    eglMakeCurrent( m_Display, m_Surface, m_Surface, m_Context);
    setCurrent();
}

void EGLContext::swapBuffers()
{
    eglSwapBuffers(m_Display, m_Surface);
}

}
