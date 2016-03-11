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
#ifdef AVG_ENABLE_RPI
    #include "BCMDisplay.h"
#else
    #include "X11Display.h"
#endif

#include "../base/Exception.h"
#include "../base/StringHelper.h"

#include <SDL2/SDL_syswm.h>
#include <EGL/egl.h>

#include <iostream>

namespace avg{

using namespace std;

EGLContext::EGLContext(const GLConfig& glConfig, const IntPoint& windowSize,
        const SDL_SysWMinfo& pSDLWMInfo)
    : GLContext(windowSize)
{
    useSDLContext(pSDLWMInfo);
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

void EGLContext::useSDLContext(const SDL_SysWMinfo& pSDLWMInfo)
{
    m_bOwnsContext = false;
    m_Context = eglGetCurrentContext();
    m_Display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
}

void EGLContext::activate()
{
    eglMakeCurrent(m_Display, m_Surface, m_Surface, m_Context);
    setCurrent();
}

void EGLContext::swapBuffers()
{
    AVG_ASSERT(false);
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
