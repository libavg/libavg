
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

#include "Player.h"

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
    m_Size = dp.m_WindowSize;
    m_Viewport = IntRect(IntPoint(0,0), dp.m_Size);
    m_bIsFullscreen = dp.m_bFullscreen;
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

const IntRect& Window::getViewport() const
{
    return m_Viewport;
}

bool Window::isFullscreen() const
{
    return m_bIsFullscreen;
}

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

void Window::setGLContext(GLContext* pGLContext)
{
    m_pGLContext = pGLContext;
}

}
