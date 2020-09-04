
//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2020 Ulrich von Zadow
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


#include "SDLWindow.h"

#include "Player.h"
#include "MouseEvent.h"
#include "MouseWheelEvent.h"
#include "SDLTouchInputDevice.h"

#include "../base/Exception.h"
#include "../base/Logger.h"
#include "../base/ScopeTimer.h"
#include "../base/OSHelper.h"
#include "../base/StringHelper.h"

#include "../graphics/GLContext.h"
#include "../graphics/GLContextManager.h"

#ifdef WIN32
#undef WIN32_LEAN_AND_MEAN
#endif
#include <SDL2/SDL_syswm.h>
#include <SDL2/SDL_events.h>

#include <iostream>

using namespace std;

namespace avg {

SDLWindow::SDLWindow(const DisplayParams& dp, const WindowParams& wp, GLConfig glConfig)
    : Window(wp, dp.isFullscreen()),
      m_pSDLWindow(0),
      m_SDLGLContext(0),
      m_LastMousePos(IntPoint(-1, -1))
{
    IntPoint pos = getPos();
    if (pos.x == -1) {
        pos.x = SDL_WINDOWPOS_UNDEFINED;
        pos.y = SDL_WINDOWPOS_UNDEFINED;
    }
    unsigned int flags = SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL;
    if (dp.isFullscreen()) {
        flags |= SDL_WINDOW_FULLSCREEN | SDL_WINDOW_BORDERLESS;
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
            AVG_LOG_ERROR("Unsupported bpp " << dp.getBPP() << "in SDLWindow::init()");
            exit(-1);
    }
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 0);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    if (glConfig.m_bGLES) {
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    }
    if (glConfig.m_bUseDebugContext && !glConfig.m_bGLES) {
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
    }

    while (glConfig.m_MultiSampleSamples && !m_SDLGLContext) {
        if (glConfig.m_MultiSampleSamples > 1) {
            SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
            SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES,
                    glConfig.m_MultiSampleSamples);
        } else {
            SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);
            SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 0);
        }
        m_pSDLWindow = SDL_CreateWindow(wp.m_sTitle.c_str(), pos.x, pos.y,
                getSize().x, getSize().y, flags);
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
    GLContext* pGLContext = 
            GLContextManager::get()->createContext(glConfig, wp.m_Size, &info);
    setGLContext(pGLContext);
    pGLContext->logConfig();
}

SDLWindow::~SDLWindow()
{
    if (m_pSDLWindow) {
        SDL_DestroyWindow(m_pSDLWindow);
    }
}

void SDLWindow::setTouchHandler(SDLTouchInputDevicePtr pInputDevice)
{
    m_pTouchInputDevice = pInputDevice;
}

bool SDLWindow::hasTouchHandler() const
{
    return (m_pTouchInputDevice != SDLTouchInputDevicePtr());
}

void SDLWindow::setTitle(const string& sTitle)
{
    SDL_SetWindowTitle(m_pSDLWindow, sTitle.c_str());
}

static ProfilingZoneID SwapBufferProfilingZone("Render - swap buffers");

void SDLWindow::swapBuffers() const
{
    ScopeTimer timer(SwapBufferProfilingZone);
    getGLContext()->activate();
#ifdef AVG_ENABLE_RPI
    SDL_GL_SwapWindow(m_pSDLWindow);
#else
    getGLContext()->swapBuffers();
#endif
    GLContext::checkError("swapBuffers()");
}

vector<EventPtr> SDLWindow::pollEvents()
{
    SDL_Event sdlEvent;
    vector<EventPtr> events;
    KeyEventPtr pPendingKeyEvent;
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
            case SDL_MOUSEWHEEL:
                {
                    glm::vec2 motion(sdlEvent.wheel.x, sdlEvent.wheel.y);
                    pNewEvent = MouseWheelEventPtr(
                            new MouseWheelEvent(m_LastMousePos, motion));
                }
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
//                cerr << "down" << endl;
                if (pPendingKeyEvent) {
                    events.push_back(pPendingKeyEvent);
                }
                pPendingKeyEvent = createKeyEvent(Event::KEY_DOWN, sdlEvent);
                break;
            case SDL_KEYUP:
//                cerr << "up" << endl;
                if (pPendingKeyEvent) {
                    events.push_back(pPendingKeyEvent);
                    pPendingKeyEvent = KeyEventPtr();
                }
                pNewEvent = createKeyEvent(Event::KEY_UP, sdlEvent);
                break;
            case SDL_TEXTINPUT:
//                cerr << "Text: " << sdlEvent.text.text << endl;
                if (pPendingKeyEvent) {
                    pPendingKeyEvent->setText(sdlEvent.text.text);
                    pNewEvent = pPendingKeyEvent;
                    pPendingKeyEvent = KeyEventPtr();
                }
                break;
            case SDL_QUIT:
                pNewEvent = EventPtr(new Event(Event::QUIT, Event::NONE));
                break;
            case SDL_FINGERMOTION:
            case SDL_FINGERDOWN:
            case SDL_FINGERUP:
                if (m_pTouchInputDevice) {
                    m_pTouchInputDevice->onTouchEvent(this, sdlEvent);
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
    if (pPendingKeyEvent) {
        events.push_back(pPendingKeyEvent);
        pPendingKeyEvent = KeyEventPtr();
    }
    if (numEvents > 124) {
        AVG_TRACE(Logger::category::EVENTS, Logger::severity::WARNING, 
                "SDL Event queue full, dropping events.");
    }
    return events;
}

void SDLWindow::setMousePos(const IntPoint& pos)
{
    SDL_WarpMouseInWindow(m_pSDLWindow, pos.x, pos.y);
}

void SDLWindow::setGamma(float red, float green, float blue)
{
    Uint16 redRamp[256];
    Uint16 greenRamp[256];
    Uint16 blueRamp[256];
    SDL_CalculateGammaRamp(red, redRamp);
    SDL_CalculateGammaRamp(green, greenRamp);
    SDL_CalculateGammaRamp(blue, blueRamp);
    int rc = SDL_SetWindowGammaRamp(m_pSDLWindow, redRamp, greenRamp, blueRamp);
    if (rc != 0) {
        AVG_LOG_INFO("Unable to set display gamma.");
    }
}

#ifdef _WIN32
HWND SDLWindow::getWinHWnd()
{
    SDL_SysWMinfo info;
    SDL_VERSION(&info.version);
    bool bOK = SDL_GetWindowWMInfo(m_pSDLWindow, &info);
    AVG_ASSERT(bOK);
    return info.info.win.window;

}
#endif

#if defined(__linux__) && !defined(AVG_ENABLE_RPI)
::Display* SDLWindow::getX11Display()
{
    SDL_SysWMinfo info;
    SDL_VERSION(&info.version);
    bool bOk = SDL_GetWindowWMInfo(m_pSDLWindow, &info);
    AVG_ASSERT(bOk);
    return info.info.x11.display;
}

::Window SDLWindow::getX11Window()
{
    SDL_SysWMinfo info;
    SDL_VERSION(&info.version);
    bool bOk = SDL_GetWindowWMInfo(m_pSDLWindow, &info);
    AVG_ASSERT(bOk);
    return info.info.x11.window;
}
#endif

EventPtr SDLWindow::createMouseEvent(Event::Type type, const SDL_Event& sdlEvent,
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

EventPtr SDLWindow::createMouseButtonEvent(Event::Type type, const SDL_Event& sdlEvent) 
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

KeyEventPtr SDLWindow::createKeyEvent(Event::Type type, const SDL_Event& sdlEvent)
{
/*
    cerr << "scancode: " << sdlEvent.key.keysym.scancode <<
            ", sym: " << sdlEvent.key.keysym.sym <<
            ", name: " << SDL_GetKeyName(sdlEvent.key.keysym.sym) <<
            ", mod: " << sdlEvent.key.keysym.mod << endl;
            */
    KeyEventPtr pEvent(new KeyEvent(type,
            sdlEvent.key.keysym.scancode, SDL_GetKeyName(sdlEvent.key.keysym.sym),
            (unsigned)sdlEvent.key.keysym.mod));
    return pEvent;
}

}
