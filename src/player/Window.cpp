
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


#include "Window.h"
#include "../avgconfigwrapper.h"

#include "../base/Exception.h"
#include "../base/Logger.h"
#include "../base/ScopeTimer.h"
#include "../base/OSHelper.h"
#include "../base/StringHelper.h"

#include "../graphics/GLContext.h"
#include "../graphics/Filterflip.h"
#include "../graphics/Filterfliprgb.h"

#include <SDL/SDL.h>
#include <SDL/SDL_syswm.h>

#include <iostream>

using namespace std;

namespace avg {

Window::Window(const DisplayParams& dp, GLConfig glConfig)
    : m_pGLContext(0)
{
    // This "fixes" the default behaviour of SDL under x11, avoiding it
    // to report relative mouse coordinates when going fullscreen and
    // the mouse cursor is hidden (grabbed). So far libavg and apps based
    // on it don't use relative coordinates.
    setEnv("SDL_MOUSE_RELATIVE", "0");

    stringstream ss;
    if (dp.m_Pos.x != -1) {
        ss << dp.m_Pos.x << "," << dp.m_Pos.y;
        setEnv("SDL_VIDEO_WINDOW_POS", ss.str().c_str());
    }
    m_Size = dp.m_WindowSize;
    unsigned int flags = 0;
    if (dp.m_bFullscreen) {
        flags |= SDL_FULLSCREEN;
    }
    m_bIsFullscreen = dp.m_bFullscreen;

    if (!dp.m_bHasWindowFrame) {
        flags |= SDL_NOFRAME;
    }

    SDL_Surface * pSDLSurface;
#ifndef linux
    if (glConfig.m_bUseDebugContext) {
        glConfig.m_bUseDebugContext = false;
    }
    switch (dp.m_BPP) {
        case 24:
            SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
            SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
            SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
            SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 24);
            break;
        case 16:
            SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
            SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 6);
            SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
            SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 16);
            break;
        default:
            AVG_LOG_ERROR("Unsupported bpp " << dp.m_BPP <<
                    "in Window::init()");
            exit(-1);
    }
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 0);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL , 0); 
    flags |= SDL_OPENGL;

    while (glConfig.m_MultiSampleSamples && !pSDLSurface) {
        if (glConfig.m_MultiSampleSamples > 1) {
            SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
            SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES,
                    glConfig.m_MultiSampleSamples);
        } else {
            SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);
            SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 0);
        }
        pSDLSurface = SDL_SetVideoMode(m_Size.x, m_Size.y, dp.m_BPP, flags);
        if (!pSDLSurface) {
            glConfig.m_MultiSampleSamples = GLContext::nextMultiSampleValue(
                    glConfig.m_MultiSampleSamples);
        }
    }
#else
    // Linux version: Context created manually, not by SDL
    pSDLSurface = SDL_SetVideoMode(m_Size.x, m_Size.y, dp.m_BPP, flags);
#endif
    if (!pSDLSurface) {
        throw Exception(AVG_ERR_UNSUPPORTED, string("Setting SDL video mode failed: ")
                + SDL_GetError() + ". (size=" + toString(m_Size) + ", bpp=" + 
                toString(dp.m_BPP) + ").");
    }
    SDL_SysWMinfo info;
    SDL_VERSION(&info.version);
    int rc = SDL_GetWMInfo(&info);
    AVG_ASSERT(rc != -1);
    m_pGLContext = GLContext::create(glConfig, m_Size, &info);

#if defined(HAVE_XI2_1) || defined(HAVE_XI2_2) 
    SDL_EventState(SDL_SYSWMEVENT, SDL_ENABLE);
    m_pXIMTInputDevice = 0;
#endif
    SDL_WM_SetCaption("libavg", 0);

    m_Size = dp.m_Size;
    m_pGLContext->logConfig();
}

Window::~Window()
{
    if (m_pGLContext) {
        delete m_pGLContext;
        m_pGLContext = 0;
    }
}

void Window::setTitle(const string& sTitle)
{
    SDL_WM_SetCaption(sTitle.c_str(), 0);
}

static ProfilingZoneID SwapBufferProfilingZone("Render - swap buffers");

void Window::swapBuffers()
{
    ScopeTimer timer(SwapBufferProfilingZone);
#ifdef linux    
    m_pGLContext->swapBuffers();
#else
    SDL_GL_SwapBuffers();
#endif
    GLContext::checkError("swapBuffers()");
}

BitmapPtr Window::screenshot(int buffer)
{
    BitmapPtr pBmp;
    glproc::BindFramebuffer(GL_FRAMEBUFFER, 0);
    if (m_pGLContext->isGLES()) {
        pBmp = BitmapPtr(new Bitmap(m_Size, R8G8B8X8, "screenshot"));
        glReadPixels(0, 0, m_Size.x, m_Size.y, GL_RGBA, GL_UNSIGNED_BYTE, 
                pBmp->getPixels());
        GLContext::checkError("Window::screenshot:glReadPixels()");
    } else {
#ifndef AVG_ENABLE_EGL
        pBmp = BitmapPtr(new Bitmap(m_Size, B8G8R8X8, "screenshot"));
        string sTmp;
        bool bBroken = getEnv("AVG_BROKEN_READBUFFER", sTmp);
        GLenum buf = buffer;
        if (!buffer) {
            if (bBroken) {
                // Workaround for buggy GL_FRONT on some machines.
                buf = GL_BACK;
            } else {
                buf = GL_FRONT;
            }
        }
        glReadBuffer(buf);
        GLContext::checkError("Window::screenshot:glReadBuffer()");
        glproc::BindBuffer(GL_PIXEL_PACK_BUFFER, 0);
        glReadPixels(0, 0, m_Size.x, m_Size.y, GL_BGRA, GL_UNSIGNED_BYTE, 
                pBmp->getPixels());
        GLContext::checkError("Window::screenshot:glReadPixels()");
#endif
    }
    FilterFlip().applyInPlace(pBmp);
    return pBmp;
}

const IntPoint& Window::getSize() const
{
    return m_Size;
}

bool Window::isFullscreen() const
{
    return m_bIsFullscreen;
}

}
