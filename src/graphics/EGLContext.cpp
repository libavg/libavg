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

#include "EGLContext.h"
#include "GLContextAttribs.h"
#ifdef AVG_ENABLE_BCM
    #include "BCMDisplay.h"
#endif
#ifdef AVG_ENABLE_X11
    #include "X11Display.h"
#endif

#include "../base/Exception.h"
#include "../base/StringHelper.h"

#include <SDL2/SDL_syswm.h>
#include <EGL/egl.h>

#include <iostream>
#include <boost/numeric/conversion/cast.hpp>

namespace avg{

using namespace std;
using boost::numeric_cast;

EGLContext::EGLContext(const GLConfig& glConfig, const IntPoint& windowSize,
        const SDL_SysWMinfo* pSDLWMInfo)
    : GLContext(windowSize)
{
    if (pSDLWMInfo) {
        useSDLContext(pSDLWMInfo);
    } else {
        createEGLContext(glConfig, windowSize);
    }
    init(glConfig, true);
}

EGLContext::~EGLContext()
{
    getPBOCache().deleteBuffers();
    deleteObjects();
    if (m_bOwnsContext) {
        eglMakeCurrent(m_Display, EGL_NO_SURFACE, EGL_NO_SURFACE, 0);
        eglDestroyContext(m_Display, m_Context);
        eglDestroySurface(m_Display, m_Surface);
        eglTerminate(m_Display);
        m_Context = 0;
    }
}

void EGLContext::useSDLContext(const SDL_SysWMinfo* pSDLWMInfo)
{
    m_bOwnsContext = false;
    m_Context = eglGetCurrentContext();
    m_Display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
}

void EGLContext::createEGLContext(const GLConfig&, const IntPoint& windowSize)
{
    m_bOwnsContext = true;

#ifdef AVG_ENABLE_BCM
    m_nativeDisplay = EGL_DEFAULT_DISPLAY;
#endif
#ifdef AVG_ENABLE_X11
    m_nativeDisplay = (EGLNativeDisplayType)getX11Display();
#endif
    m_Display = eglGetDisplay(m_nativeDisplay);
    checkEGLError(m_Display == EGL_NO_DISPLAY, "No EGL display available");

    EGLBoolean bOk = eglInitialize(m_Display, NULL, NULL);
    checkEGLError(!bOk, "eglInitialize failed");

    GLContextAttribs fbAttrs;
    fbAttrs.append(EGL_RED_SIZE, 1);
    fbAttrs.append(EGL_GREEN_SIZE, 1);
    fbAttrs.append(EGL_BLUE_SIZE, 1);
    fbAttrs.append(EGL_DEPTH_SIZE, 0);
    fbAttrs.append(EGL_STENCIL_SIZE, 1);
#ifdef AVG_ENABLE_BCM
    int alphaSize = 1;
#endif
#ifdef AVG_ENABLE_X11
    int alphaSize = 0;
#endif
    fbAttrs.append(EGL_ALPHA_SIZE, alphaSize);
    fbAttrs.append(EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT);
    EGLint numFBConfig;
    EGLConfig config;
    bOk = eglChooseConfig(m_Display, fbAttrs.get(), &config, 1, &numFBConfig);
    checkEGLError(!bOk, "Failed to choose EGL config");

    EGLint vid;
    bOk = eglGetConfigAttrib(m_Display, config, EGL_NATIVE_VISUAL_ID, &vid);
    AVG_ASSERT(bOk);

    if (!eglBindAPI(EGL_OPENGL_ES_API)) {
        cerr << "Failed to bind GLES API to EGL\n";
        return;
    }

    if (numFBConfig != 1) {
        cerr << "Didn't get exactly one config, but " << numFBConfig << endl;
        return;
    }

#ifdef AVG_ENABLE_BCM
    m_Surface = createBCMPixmapSurface(m_Display, config);
#endif
#ifdef AVG_ENABLE_X11
    XVisualInfo visTemplate, *pVisualInfo;
    visTemplate.screen = 0;
    int numVisuals;
    pVisualInfo = XGetVisualInfo((_XDisplay*)m_nativeDisplay, VisualScreenMask,
            &visTemplate, & numVisuals);
    AVG_ASSERT(pVisualInfo);

    Pixmap pmp = XCreatePixmap(
            (_XDisplay*)m_nativeDisplay,
            RootWindow((_XDisplay*)m_nativeDisplay, pVisualInfo[0].screen),
            8, 8,
            numeric_cast<unsigned int>(pVisualInfo[0].depth));

    m_Surface = eglCreatePixmapSurface(m_Display, config, (EGLNativePixmapType)pmp,
            NULL);
#endif
    
    //dumpEGLConfig(config);
    AVG_ASSERT(m_Surface);

    GLContextAttribs attrs;
    attrs.append(EGL_CONTEXT_CLIENT_VERSION, 2);
    m_Context = eglCreateContext(m_Display, config, NULL, attrs.get());
    checkEGLError(!m_Context, "Unable to create EGL context");
}

void EGLContext::activate()
{
    eglMakeCurrent(m_Display, m_Surface, m_Surface, m_Context);
    setCurrent();
}

void EGLContext::swapBuffers()
{
    eglSwapBuffers(m_Display, m_Surface);
}

void EGLContext::checkEGLError(bool bError, const std::string& sMsg)
{
    if (bError) {
        throw Exception(AVG_ERR_VIDEO_INIT_FAILED, sMsg + " (EGL error: " +
                toString(eglGetError()) + ")");
    }
}

void EGLContext::dumpEGLConfig(const EGLConfig& config) const
{
    cout << "EGL configuration:" << endl;
    dumpEGLConfigAttrib(config, EGL_RED_SIZE, "RED_SIZE");
    dumpEGLConfigAttrib(config, EGL_GREEN_SIZE, "GREEN_SIZE");
    dumpEGLConfigAttrib(config, EGL_BLUE_SIZE, "BLUE_SIZE");
    dumpEGLConfigAttrib(config, EGL_ALPHA_SIZE, "ALPHA_SIZE");
    dumpEGLConfigAttrib(config, EGL_BUFFER_SIZE, "BUFFER_SIZE");
    dumpEGLConfigAttrib(config, EGL_DEPTH_SIZE, "DEPTH_SIZE");
    dumpEGLConfigAttrib(config, EGL_STENCIL_SIZE, "STENCIL_SIZE");
}

void EGLContext::dumpEGLConfigAttrib(const EGLConfig& config, EGLint attrib,
        const string& name) const
{
    EGLint value;
    if (eglGetConfigAttrib(m_Display, config, attrib, &value)) {
        cout << "  " << name << ": " << value << endl;
    }
    else {
        cerr << "  Failed to get EGL config attribute " << name << endl;
    }
}

}
