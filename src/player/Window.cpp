
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
#if defined(HAVE_XI2_1) || defined(HAVE_XI2_2) 
#include "XInputMTInputDevice.h" 
#endif

#include "../base/Exception.h"
#include "../base/Logger.h"
#include "../base/ScopeTimer.h"
#include "../base/OSHelper.h"
#include "../base/StringHelper.h"

#include "../graphics/GLContext.h"
#include "../graphics/GLContextManager.h"
#include "../graphics/Filterflip.h"
#include "../graphics/Filterfliprgb.h"
#include "../graphics/ImageCache.h"

#ifdef WIN32
#undef WIN32_LEAN_AND_MEAN
#endif
#include <SDL2/SDL_syswm.h>
#include <SDL2/SDL_events.h>

#include <iostream>

using namespace std;

namespace avg {

Window::Window(const DisplayParams& dp, const WindowParams& wp, GLConfig glConfig)
    : m_pSDLWindow(0),
      m_SDLGLContext(0),
      m_LastMousePos(IntPoint(-1, -1)),
      m_pGLContext(0)
{
    m_Pos = wp.m_Pos;
    m_Size = wp.m_Size;
    m_Viewport = wp.m_Viewport;
    m_bIsFullscreen = dp.isFullscreen();

    if (m_Pos.x == -1) {
        m_Pos.x = SDL_WINDOWPOS_UNDEFINED;
        m_Pos.y = SDL_WINDOWPOS_UNDEFINED;
    }

    unsigned int flags = SDL_WINDOW_OPENGL;
    if (dp.isFullscreen()) {
        flags |= SDL_WINDOW_FULLSCREEN;
    }
    if (!wp.m_bHasWindowFrame) {
        flags |= SDL_WINDOW_BORDERLESS;
    }

    switch (dp.getBPP()) {
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
            AVG_LOG_ERROR("Unsupported bpp " << dp.getBPP() << "in Window::init()");
            exit(-1);
    }
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 0);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    if (glConfig.m_bGLES) {
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    }

    setEnv("DISPLAY", ":0."+toString(wp.m_DisplayServer));
    while (glConfig.m_MultiSampleSamples && !m_SDLGLContext) {
        if (glConfig.m_MultiSampleSamples > 1) {
            SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
            SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES,
                    glConfig.m_MultiSampleSamples);
        } else {
            SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);
            SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 0);
        }
        m_pSDLWindow = SDL_CreateWindow("libavg", m_Pos.x, m_Pos.y, m_Size.x, m_Size.y,
                flags);
        if (m_pSDLWindow) {
            m_SDLGLContext = SDL_GL_CreateContext(m_pSDLWindow);
        } else {
            m_SDLGLContext = 0;
        }
        if (!m_SDLGLContext) {
            glConfig.m_MultiSampleSamples = GLContext::nextMultiSampleValue(
                    glConfig.m_MultiSampleSamples);
        }
    }

#ifndef __linux__
    glConfig.m_bUseDebugContext = false;
#endif

    if (!m_SDLGLContext) {
        throw Exception(AVG_ERR_UNSUPPORTED, string("Creating window failed: ")
                + SDL_GetError() + ". (size=" + toString(wp.m_Size) + ", bpp=" +
                toString(dp.getBPP()) + ").");
    }
    SDL_GL_MakeCurrent(m_pSDLWindow, m_SDLGLContext);
    SDL_SysWMinfo info;
    SDL_VERSION(&info.version);
    int rc = SDL_GetWindowWMInfo(m_pSDLWindow, &info);
    AVG_ASSERT(rc != -1);
    m_pGLContext = GLContextManager::get()->createContext(glConfig, wp.m_Size, &info);
    m_pGLContext->logConfig();

/*
#if defined(HAVE_XI2_1) || defined(HAVE_XI2_2) 
    SDL_EventState(SDL_SYSWMEVENT, SDL_ENABLE);
    m_pXIMTInputDevice = 0;
#endif
*/    
}

Window::~Window()
{
    if (m_pSDLWindow) {
        SDL_DestroyWindow(m_pSDLWindow);
    }
    if (m_pGLContext) {
        delete m_pGLContext;
        m_pGLContext = 0;
    }
}

void Window::setTitle(const string& sTitle)
{
    SDL_SetWindowTitle(m_pSDLWindow, sTitle.c_str());
}

static ProfilingZoneID SwapBufferProfilingZone("Render - SDL swap buffers");

void Window::swapBuffers() const
{
    ScopeTimer timer(SwapBufferProfilingZone);
#ifdef __linux__    
    m_pGLContext->swapBuffers();
#else
    SDL_GL_SwapBuffers();
#endif
    GLContext::checkError("swapBuffers()");
}

BitmapPtr Window::screenshot(int buffer)
{
    AVG_ASSERT(m_pGLContext);
    m_pGLContext->activate();
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

const IntPoint& Window::getPos() const
{
    return m_Pos;
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

GLContext* Window::getGLContext() const
{
    AVG_ASSERT(m_pGLContext);
    return m_pGLContext;
}

vector<EventPtr> Window::pollEvents()
{
    SDL_Event sdlEvent;
    vector<EventPtr> events;

    int numEvents = 0;
    while (SDL_PollEvent(&sdlEvent)) {
        numEvents++;
        EventPtr pNewEvent;
        switch (sdlEvent.type) {
            case SDL_MOUSEMOTION:
                {
                    pNewEvent = createMouseEvent(Event::CURSOR_MOTION, sdlEvent, 
                            MouseEvent::NO_BUTTON);
                    CursorEventPtr pNewCursorEvent = 
                            boost::dynamic_pointer_cast<CursorEvent>(pNewEvent);
                    if (!events.empty()) {
                        CursorEventPtr pLastEvent = 
                                boost::dynamic_pointer_cast<CursorEvent>(events.back());
                        if (pLastEvent && *pNewCursorEvent == *pLastEvent) {
                            pNewEvent = EventPtr();
                        }
                    }
                }
                break;
            case SDL_MOUSEBUTTONDOWN:
                pNewEvent = createMouseButtonEvent(Event::CURSOR_DOWN, sdlEvent);
                break;
            case SDL_MOUSEBUTTONUP:
                pNewEvent = createMouseButtonEvent(Event::CURSOR_UP, sdlEvent);
                break;
            case SDL_JOYAXISMOTION:
//                pNewEvent = createAxisEvent(sdlEvent));
                break;
            case SDL_JOYBUTTONDOWN:
//                pNewEvent = createButtonEvent(Event::BUTTON_DOWN, sdlEvent));
                break;
            case SDL_JOYBUTTONUP:
//                pNewEvent = createButtonEvent(Event::BUTTON_UP, sdlEvent));
                break;
            case SDL_KEYDOWN:
                pNewEvent = createKeyEvent(Event::KEY_DOWN, sdlEvent);
                break;
            case SDL_KEYUP:
                pNewEvent = createKeyEvent(Event::KEY_UP, sdlEvent);
                break;
            case SDL_QUIT:
                pNewEvent = EventPtr(new Event(Event::QUIT, Event::NONE));
                break;
            case SDL_SYSWMEVENT:
                {
#if defined(HAVE_XI2_1) || defined(HAVE_XI2_2) 
                    SDL_SysWMmsg* pMsg = sdlEvent.syswm.msg;
                    AVG_ASSERT(pMsg->subsystem == SDL_SYSWM_X11);
                    if (m_pXIMTInputDevice) {
                        m_pXIMTInputDevice->handleXIEvent(pMsg->msg.x11.event);
                    }
#endif
                }
                break;
            default:
                // Ignore unknown events.
                break;
        }
        if (pNewEvent) {
            events.push_back(pNewEvent);
        }
    }
    if (numEvents > 124) {
        AVG_TRACE(Logger::category::EVENTS, Logger::severity::WARNING, 
                "SDL Event queue full, dropping events.");
    }
    return events;
}

void Window::setXIMTInputDevice(XInputMTInputDevice* pInputDevice)
{
    AVG_ASSERT(!m_pXIMTInputDevice);
    m_pXIMTInputDevice = pInputDevice;
}

bool Window::setGamma(float red, float green, float blue)
{
    Uint16 redRamp[256];
    Uint16 greenRamp[256];
    Uint16 blueRamp[256];
    SDL_CalculateGammaRamp(red, redRamp);
    SDL_CalculateGammaRamp(green, greenRamp);
    SDL_CalculateGammaRamp(blue, blueRamp);
    int rc = SDL_SetWindowGammaRamp(m_pSDLWindow, redRamp, greenRamp, blueRamp);
    return rc == 0;
}

EventPtr Window::createMouseEvent(Event::Type type, const SDL_Event& sdlEvent,
        long button)
{
    int x, y;
    Uint8 buttonState = SDL_GetMouseState(&x, &y);
    IntRect viewport = getViewport();
    IntPoint size = getSize();
    x = int((x*viewport.width())/size.x);
    y = int((y*viewport.height())/size.y);
    glm::vec2 speed;
    if (m_LastMousePos.x == -1) {
        speed = glm::vec2(0,0);
    } else {
        float lastFrameTime = 1000/Player::get()->getEffectiveFramerate();
        speed = glm::vec2(x-m_LastMousePos.x, y-m_LastMousePos.y)/lastFrameTime;
    }
    MouseEventPtr pEvent(new MouseEvent(type, (buttonState & SDL_BUTTON(1)) != 0,
            (buttonState & SDL_BUTTON(2)) != 0, (buttonState & SDL_BUTTON(3)) != 0,
            IntPoint(x, y), button, speed));

    m_LastMousePos = pEvent->getPos();
    return pEvent; 
}

EventPtr Window::createMouseButtonEvent(Event::Type type, const SDL_Event& sdlEvent) 
{
    long button = 0;
    switch (sdlEvent.button.button) {
        case SDL_BUTTON_LEFT:
            button = MouseEvent::LEFT_BUTTON;
            break;
        case SDL_BUTTON_MIDDLE:
            button = MouseEvent::MIDDLE_BUTTON;
            break;
        case SDL_BUTTON_RIGHT:
            button = MouseEvent::RIGHT_BUTTON;
            break;
        default:
            AVG_ASSERT(false);
    }
    return createMouseEvent(type, sdlEvent, button);
 
}

EventPtr Window::createKeyEvent(Event::Type type, const SDL_Event& sdlEvent)
{
    KeyEventPtr pEvent(new KeyEvent(type,
            sdlEvent.key.keysym.scancode, sdlEvent.key.keysym.sym,
            SDL_GetKeyName(sdlEvent.key.keysym.sym), (unsigned)sdlEvent.key.keysym.mod));
    return pEvent;
}

}
