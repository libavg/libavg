
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


#include "SDLWindow.h"
#include "../avgconfigwrapper.h"

#include "Player.h"
#include "MouseEvent.h"
#include "KeyEvent.h"
#if defined(HAVE_XI2_1) || defined(HAVE_XI2_2) 
#include "XInputMTInputDevice.h" 
#endif

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

vector<long> SDLWindow::s_KeyCodeTranslationTable(SDLK_LAST, key::KEY_UNKNOWN);

SDLWindow::SDLWindow(const DisplayParams& dp, GLConfig glConfig)
    : Window(dp, glConfig),
      m_pLastMouseEvent(new MouseEvent(Event::CURSOR_MOTION, false, false, false, 
            IntPoint(-1, -1), MouseEvent::NO_BUTTON, glm::vec2(-1, -1), 0))
{
    initTranslationTable();

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
    unsigned int flags = 0;
    if (dp.m_bFullscreen) {
        flags |= SDL_FULLSCREEN;
    }
    if (!dp.m_bHasWindowFrame) {
        flags |= SDL_NOFRAME;
    }

    SDL_Surface * pSDLSurface;
    IntPoint size = dp.m_WindowSize;
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
        pSDLSurface = SDL_SetVideoMode(size.x, size.y, dp.m_BPP, flags);
        if (!pSDLSurface) {
            glConfig.m_MultiSampleSamples = GLContext::nextMultiSampleValue(
                    glConfig.m_MultiSampleSamples);
        }
    }
#else
    // Linux version: Context created manually, not by SDL
    pSDLSurface = SDL_SetVideoMode(size.x, size.y, dp.m_BPP, flags);
#endif
    if (!pSDLSurface) {
        throw Exception(AVG_ERR_UNSUPPORTED, string("Setting SDL video mode failed: ")
                + SDL_GetError() + ". (size=" + toString(size) + ", bpp=" + 
                toString(dp.m_BPP) + ").");
    }
    SDL_SysWMinfo info;
    SDL_VERSION(&info.version);
    int rc = SDL_GetWMInfo(&info);
    AVG_ASSERT(rc != -1);
    GLContext* pGLContext = GLContext::create(glConfig, size, &info);
    setGLContext(pGLContext);

#if defined(HAVE_XI2_1) || defined(HAVE_XI2_2) 
    SDL_EventState(SDL_SYSWMEVENT, SDL_ENABLE);
    m_pXIMTInputDevice = 0;
#endif
    SDL_WM_SetCaption("libavg", 0);

    pGLContext->logConfig();
}

SDLWindow::~SDLWindow()
{
}

void SDLWindow::setTitle(const string& sTitle)
{
    SDL_WM_SetCaption(sTitle.c_str(), 0);
}

static ProfilingZoneID SwapBufferProfilingZone("Render - swap buffers");

void SDLWindow::swapBuffers() const
{
    ScopeTimer timer(SwapBufferProfilingZone);
#ifdef linux    
    getGLContext()->swapBuffers();
#else
    SDL_GL_SwapBuffers();
#endif
    GLContext::checkError("swapBuffers()");
}

const char * getEventTypeName(unsigned char type) 
{
    switch (type) {
            case SDL_ACTIVEEVENT:
                return "SDL_ACTIVEEVENT";
            case SDL_KEYDOWN:
                return "SDL_KEYDOWN";
            case SDL_KEYUP:
                return "SDL_KEYUP";
            case SDL_MOUSEMOTION:
                return "SDL_MOUSEMOTION";
            case SDL_MOUSEBUTTONDOWN:
                return "SDL_MOUSEBUTTONDOWN";
            case SDL_MOUSEBUTTONUP:
                return "SDL_MOUSEBUTTONUP";
            case SDL_JOYAXISMOTION:
                return "SDL_JOYAXISMOTION";
            case SDL_JOYBUTTONDOWN:
                return "SDL_JOYBUTTONDOWN";
            case SDL_JOYBUTTONUP:
                return "SDL_JOYBUTTONUP";
            case SDL_VIDEORESIZE:
                return "SDL_VIDEORESIZE";
            case SDL_VIDEOEXPOSE:
                return "SDL_VIDEOEXPOSE";
            case SDL_QUIT:
                return "SDL_QUIT";
            case SDL_USEREVENT:
                return "SDL_USEREVENT";
            case SDL_SYSWMEVENT:
                return "SDL_SYSWMEVENT";
            default:
                return "Unknown SDL event type";
    }
}

vector<EventPtr> SDLWindow::pollEvents()
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
            case SDL_VIDEORESIZE:
                break;
            case SDL_SYSWMEVENT:
                {
#if defined(HAVE_XI2_1) || defined(HAVE_XI2_2) 
                    SDL_SysWMmsg* pMsg = sdlEvent.syswm.msg;
                    AVG_ASSERT(pMsg->subsystem == SDL_SYSWM_X11);
                    if (m_pXIMTInputDevice) {
                        m_pXIMTInputDevice->handleXIEvent(pMsg->event.xevent);
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

void SDLWindow::setXIMTInputDevice(XInputMTInputDevice* pInputDevice)
{
    AVG_ASSERT(!m_pXIMTInputDevice);
    m_pXIMTInputDevice = pInputDevice;
}

EventPtr SDLWindow::createMouseEvent(Event::Type type, const SDL_Event& sdlEvent,
        long button)
{
    int x, y;
    Uint8 buttonState = SDL_GetMouseState(&x, &y);
    IntRect viewport = getViewport();
    IntPoint size = getSize();
    x = int((x*viewport.width())/size.x);
    y = int((y*viewport.height())/size.y);
    glm::vec2 lastMousePos = m_pLastMouseEvent->getPos();
    glm::vec2 speed;
    if (lastMousePos.x == -1) {
        speed = glm::vec2(0,0);
    } else {
        float lastFrameTime = 1000/Player::get()->getEffectiveFramerate();
        speed = glm::vec2(x-lastMousePos.x, y-lastMousePos.y)/lastFrameTime;
    }
    MouseEventPtr pEvent(new MouseEvent(type, (buttonState & SDL_BUTTON(1)) != 0,
            (buttonState & SDL_BUTTON(2)) != 0, (buttonState & SDL_BUTTON(3)) != 0,
            IntPoint(x, y), button, speed));

    m_pLastMouseEvent = pEvent;
    return pEvent; 
}

EventPtr SDLWindow::createMouseButtonEvent(Event::Type type, 
        const SDL_Event& sdlEvent) 
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
        case SDL_BUTTON_WHEELUP:
            button = MouseEvent::WHEELUP_BUTTON;
            break;
        case SDL_BUTTON_WHEELDOWN:
            button = MouseEvent::WHEELDOWN_BUTTON;
            break;
    }
    return createMouseEvent(type, sdlEvent, button);
 
}

/*
EventPtr SDLWindow::createAxisEvent(const SDL_Event & sdlEvent)
{
    return new AxisEvent(sdlEvent.jaxis.which, sdlEvent.jaxis.axis,
                sdlEvent.jaxis.value);
}


EventPtr SDLWindow::createButtonEvent
        (Event::Type type, const SDL_Event & sdlEvent) 
{
    return new ButtonEvent(type, sdlEvent.jbutton.which,
                sdlEvent.jbutton.button));
}
*/

EventPtr SDLWindow::createKeyEvent(Event::Type type, const SDL_Event& sdlEvent)
{
    long keyCode = s_KeyCodeTranslationTable[sdlEvent.key.keysym.sym];
    unsigned int modifiers = key::KEYMOD_NONE;

    if (sdlEvent.key.keysym.mod & KMOD_LSHIFT) 
        { modifiers |= key::KEYMOD_LSHIFT; }
    if (sdlEvent.key.keysym.mod & KMOD_RSHIFT) 
        { modifiers |= key::KEYMOD_RSHIFT; }
    if (sdlEvent.key.keysym.mod & KMOD_LCTRL) 
        { modifiers |= key::KEYMOD_LCTRL; }
    if (sdlEvent.key.keysym.mod & KMOD_RCTRL) 
        { modifiers |= key::KEYMOD_RCTRL; }
    if (sdlEvent.key.keysym.mod & KMOD_LALT) 
        { modifiers |= key::KEYMOD_LALT; }
    if (sdlEvent.key.keysym.mod & KMOD_RALT) 
        { modifiers |= key::KEYMOD_RALT; }
    if (sdlEvent.key.keysym.mod & KMOD_LMETA) 
        { modifiers |= key::KEYMOD_LMETA; }
    if (sdlEvent.key.keysym.mod & KMOD_RMETA) 
        { modifiers |= key::KEYMOD_RMETA; }
    if (sdlEvent.key.keysym.mod & KMOD_NUM) 
        { modifiers |= key::KEYMOD_NUM; }
    if (sdlEvent.key.keysym.mod & KMOD_CAPS) 
        { modifiers |= key::KEYMOD_CAPS; }
    if (sdlEvent.key.keysym.mod & KMOD_MODE) 
        { modifiers |= key::KEYMOD_MODE; }
    if (sdlEvent.key.keysym.mod & KMOD_RESERVED) 
        { modifiers |= key::KEYMOD_RESERVED; }

    KeyEventPtr pEvent(new KeyEvent(type,
            sdlEvent.key.keysym.scancode, keyCode,
            SDL_GetKeyName(sdlEvent.key.keysym.sym), sdlEvent.key.keysym.unicode,
                    modifiers));
    return pEvent;
}

void SDLWindow::initTranslationTable()
{
#define TRANSLATION_ENTRY(x) s_KeyCodeTranslationTable[SDLK_##x] = key::KEY_##x;

    TRANSLATION_ENTRY(UNKNOWN);
    TRANSLATION_ENTRY(BACKSPACE);
    TRANSLATION_ENTRY(TAB);
    TRANSLATION_ENTRY(CLEAR);
    TRANSLATION_ENTRY(RETURN);
    TRANSLATION_ENTRY(PAUSE);
    TRANSLATION_ENTRY(ESCAPE);
    TRANSLATION_ENTRY(SPACE);
    TRANSLATION_ENTRY(EXCLAIM);
    TRANSLATION_ENTRY(QUOTEDBL);
    TRANSLATION_ENTRY(HASH);
    TRANSLATION_ENTRY(DOLLAR);
    TRANSLATION_ENTRY(AMPERSAND);
    TRANSLATION_ENTRY(QUOTE);
    TRANSLATION_ENTRY(LEFTPAREN);
    TRANSLATION_ENTRY(RIGHTPAREN);
    TRANSLATION_ENTRY(ASTERISK);
    TRANSLATION_ENTRY(PLUS);
    TRANSLATION_ENTRY(COMMA);
    TRANSLATION_ENTRY(MINUS);
    TRANSLATION_ENTRY(PERIOD);
    TRANSLATION_ENTRY(SLASH);
    TRANSLATION_ENTRY(0);
    TRANSLATION_ENTRY(1);
    TRANSLATION_ENTRY(2);
    TRANSLATION_ENTRY(3);
    TRANSLATION_ENTRY(4);
    TRANSLATION_ENTRY(5);
    TRANSLATION_ENTRY(6);
    TRANSLATION_ENTRY(7);
    TRANSLATION_ENTRY(8);
    TRANSLATION_ENTRY(9);
    TRANSLATION_ENTRY(COLON);
    TRANSLATION_ENTRY(SEMICOLON);
    TRANSLATION_ENTRY(LESS);
    TRANSLATION_ENTRY(EQUALS);
    TRANSLATION_ENTRY(GREATER);
    TRANSLATION_ENTRY(QUESTION);
    TRANSLATION_ENTRY(AT);
    TRANSLATION_ENTRY(LEFTBRACKET);
    TRANSLATION_ENTRY(BACKSLASH);
    TRANSLATION_ENTRY(RIGHTBRACKET);
    TRANSLATION_ENTRY(CARET);
    TRANSLATION_ENTRY(UNDERSCORE);
    TRANSLATION_ENTRY(BACKQUOTE);
    TRANSLATION_ENTRY(a);
    TRANSLATION_ENTRY(b);
    TRANSLATION_ENTRY(c);
    TRANSLATION_ENTRY(d);
    TRANSLATION_ENTRY(e);
    TRANSLATION_ENTRY(f);
    TRANSLATION_ENTRY(g);
    TRANSLATION_ENTRY(h);
    TRANSLATION_ENTRY(i);
    TRANSLATION_ENTRY(j);
    TRANSLATION_ENTRY(k);
    TRANSLATION_ENTRY(l);
    TRANSLATION_ENTRY(m);
    TRANSLATION_ENTRY(n);
    TRANSLATION_ENTRY(o);
    TRANSLATION_ENTRY(p);
    TRANSLATION_ENTRY(q);
    TRANSLATION_ENTRY(r);
    TRANSLATION_ENTRY(s);
    TRANSLATION_ENTRY(t);
    TRANSLATION_ENTRY(u);
    TRANSLATION_ENTRY(v);
    TRANSLATION_ENTRY(w);
    TRANSLATION_ENTRY(x);
    TRANSLATION_ENTRY(y);
    TRANSLATION_ENTRY(z);
    TRANSLATION_ENTRY(DELETE);
    TRANSLATION_ENTRY(WORLD_0);
    TRANSLATION_ENTRY(WORLD_1);
    TRANSLATION_ENTRY(WORLD_2);
    TRANSLATION_ENTRY(WORLD_3);
    TRANSLATION_ENTRY(WORLD_4);
    TRANSLATION_ENTRY(WORLD_5);
    TRANSLATION_ENTRY(WORLD_6);
    TRANSLATION_ENTRY(WORLD_7);
    TRANSLATION_ENTRY(WORLD_8);
    TRANSLATION_ENTRY(WORLD_9);
    TRANSLATION_ENTRY(WORLD_10);
    TRANSLATION_ENTRY(WORLD_11);
    TRANSLATION_ENTRY(WORLD_12);
    TRANSLATION_ENTRY(WORLD_13);
    TRANSLATION_ENTRY(WORLD_14);
    TRANSLATION_ENTRY(WORLD_15);
    TRANSLATION_ENTRY(WORLD_16);
    TRANSLATION_ENTRY(WORLD_17);
    TRANSLATION_ENTRY(WORLD_18);
    TRANSLATION_ENTRY(WORLD_19);
    TRANSLATION_ENTRY(WORLD_20);
    TRANSLATION_ENTRY(WORLD_21);
    TRANSLATION_ENTRY(WORLD_22);
    TRANSLATION_ENTRY(WORLD_23);
    TRANSLATION_ENTRY(WORLD_24);
    TRANSLATION_ENTRY(WORLD_25);
    TRANSLATION_ENTRY(WORLD_26);
    TRANSLATION_ENTRY(WORLD_27);
    TRANSLATION_ENTRY(WORLD_28);
    TRANSLATION_ENTRY(WORLD_29);
    TRANSLATION_ENTRY(WORLD_30);
    TRANSLATION_ENTRY(WORLD_31);
    TRANSLATION_ENTRY(WORLD_32);
    TRANSLATION_ENTRY(WORLD_33);
    TRANSLATION_ENTRY(WORLD_34);
    TRANSLATION_ENTRY(WORLD_35);
    TRANSLATION_ENTRY(WORLD_36);
    TRANSLATION_ENTRY(WORLD_37);
    TRANSLATION_ENTRY(WORLD_38);
    TRANSLATION_ENTRY(WORLD_39);
    TRANSLATION_ENTRY(WORLD_40);
    TRANSLATION_ENTRY(WORLD_41);
    TRANSLATION_ENTRY(WORLD_42);
    TRANSLATION_ENTRY(WORLD_43);
    TRANSLATION_ENTRY(WORLD_44);
    TRANSLATION_ENTRY(WORLD_45);
    TRANSLATION_ENTRY(WORLD_46);
    TRANSLATION_ENTRY(WORLD_47);
    TRANSLATION_ENTRY(WORLD_48);
    TRANSLATION_ENTRY(WORLD_49);
    TRANSLATION_ENTRY(WORLD_50);
    TRANSLATION_ENTRY(WORLD_51);
    TRANSLATION_ENTRY(WORLD_52);
    TRANSLATION_ENTRY(WORLD_53);
    TRANSLATION_ENTRY(WORLD_54);
    TRANSLATION_ENTRY(WORLD_55);
    TRANSLATION_ENTRY(WORLD_56);
    TRANSLATION_ENTRY(WORLD_57);
    TRANSLATION_ENTRY(WORLD_58);
    TRANSLATION_ENTRY(WORLD_59);
    TRANSLATION_ENTRY(WORLD_60);
    TRANSLATION_ENTRY(WORLD_61);
    TRANSLATION_ENTRY(WORLD_62);
    TRANSLATION_ENTRY(WORLD_63);
    TRANSLATION_ENTRY(WORLD_64);
    TRANSLATION_ENTRY(WORLD_65);
    TRANSLATION_ENTRY(WORLD_66);
    TRANSLATION_ENTRY(WORLD_67);
    TRANSLATION_ENTRY(WORLD_68);
    TRANSLATION_ENTRY(WORLD_69);
    TRANSLATION_ENTRY(WORLD_70);
    TRANSLATION_ENTRY(WORLD_71);
    TRANSLATION_ENTRY(WORLD_72);
    TRANSLATION_ENTRY(WORLD_73);
    TRANSLATION_ENTRY(WORLD_74);
    TRANSLATION_ENTRY(WORLD_75);
    TRANSLATION_ENTRY(WORLD_76);
    TRANSLATION_ENTRY(WORLD_77);
    TRANSLATION_ENTRY(WORLD_78);
    TRANSLATION_ENTRY(WORLD_79);
    TRANSLATION_ENTRY(WORLD_80);
    TRANSLATION_ENTRY(WORLD_81);
    TRANSLATION_ENTRY(WORLD_82);
    TRANSLATION_ENTRY(WORLD_83);
    TRANSLATION_ENTRY(WORLD_84);
    TRANSLATION_ENTRY(WORLD_85);
    TRANSLATION_ENTRY(WORLD_86);
    TRANSLATION_ENTRY(WORLD_87);
    TRANSLATION_ENTRY(WORLD_88);
    TRANSLATION_ENTRY(WORLD_89);
    TRANSLATION_ENTRY(WORLD_90);
    TRANSLATION_ENTRY(WORLD_91);
    TRANSLATION_ENTRY(WORLD_92);
    TRANSLATION_ENTRY(WORLD_93);
    TRANSLATION_ENTRY(WORLD_94);
    TRANSLATION_ENTRY(WORLD_95);
    TRANSLATION_ENTRY(KP0);
    TRANSLATION_ENTRY(KP1);
    TRANSLATION_ENTRY(KP2);
    TRANSLATION_ENTRY(KP3);
    TRANSLATION_ENTRY(KP4);
    TRANSLATION_ENTRY(KP5);
    TRANSLATION_ENTRY(KP6);
    TRANSLATION_ENTRY(KP7);
    TRANSLATION_ENTRY(KP8);
    TRANSLATION_ENTRY(KP9);
    TRANSLATION_ENTRY(KP_PERIOD);
    TRANSLATION_ENTRY(KP_DIVIDE);
    TRANSLATION_ENTRY(KP_MULTIPLY);
    TRANSLATION_ENTRY(KP_MINUS);
    TRANSLATION_ENTRY(KP_PLUS);
    TRANSLATION_ENTRY(KP_ENTER);
    TRANSLATION_ENTRY(KP_EQUALS);
    TRANSLATION_ENTRY(UP);
    TRANSLATION_ENTRY(DOWN);
    TRANSLATION_ENTRY(RIGHT);
    TRANSLATION_ENTRY(LEFT);
    TRANSLATION_ENTRY(INSERT);
    TRANSLATION_ENTRY(HOME);
    TRANSLATION_ENTRY(END);
    TRANSLATION_ENTRY(PAGEUP);
    TRANSLATION_ENTRY(PAGEDOWN);
    TRANSLATION_ENTRY(F1);
    TRANSLATION_ENTRY(F2);
    TRANSLATION_ENTRY(F3);
    TRANSLATION_ENTRY(F4);
    TRANSLATION_ENTRY(F5);
    TRANSLATION_ENTRY(F6);
    TRANSLATION_ENTRY(F7);
    TRANSLATION_ENTRY(F8);
    TRANSLATION_ENTRY(F9);
    TRANSLATION_ENTRY(F10);
    TRANSLATION_ENTRY(F11);
    TRANSLATION_ENTRY(F12);
    TRANSLATION_ENTRY(F13);
    TRANSLATION_ENTRY(F14);
    TRANSLATION_ENTRY(F15);
    TRANSLATION_ENTRY(NUMLOCK);
    TRANSLATION_ENTRY(CAPSLOCK);
    TRANSLATION_ENTRY(SCROLLOCK);
    TRANSLATION_ENTRY(RSHIFT);
    TRANSLATION_ENTRY(LSHIFT);
    TRANSLATION_ENTRY(RCTRL);
    TRANSLATION_ENTRY(LCTRL);
    TRANSLATION_ENTRY(RALT);
    TRANSLATION_ENTRY(LALT);
    TRANSLATION_ENTRY(RMETA);
    TRANSLATION_ENTRY(LMETA);
    TRANSLATION_ENTRY(LSUPER);
    TRANSLATION_ENTRY(RSUPER);
    TRANSLATION_ENTRY(MODE);
    TRANSLATION_ENTRY(COMPOSE);
    TRANSLATION_ENTRY(HELP);
    TRANSLATION_ENTRY(PRINT);
    TRANSLATION_ENTRY(SYSREQ);
    TRANSLATION_ENTRY(BREAK);
    TRANSLATION_ENTRY(MENU);
    TRANSLATION_ENTRY(POWER);
    TRANSLATION_ENTRY(EURO);
    TRANSLATION_ENTRY(UNDO);
}

}
