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


#include "SDLDisplayEngine.h"
#include "../avgconfigwrapper.h"

#ifdef __APPLE__
#include "SDLMain.h"
#endif

#include "Shape.h"

#include "Event.h"
#include "MouseEvent.h"
#include "KeyEvent.h"
#include "Window.h"

#include "../base/MathHelper.h"
#include "../base/Exception.h"
#include "../base/Logger.h"
#include "../base/ScopeTimer.h"
#include "../base/OSHelper.h"
#include "../base/StringHelper.h"

#include "../graphics/GLContext.h"
#include "../graphics/Filterflip.h"
#include "../graphics/Filterfliprgb.h"
#include "../graphics/Display.h"

#include "../video/VideoDecoder.h"

#include "OGLSurface.h"
#include "OffscreenCanvas.h"

#ifdef __APPLE__
#include <ApplicationServices/ApplicationServices.h>
#endif

#include <SDL/SDL.h>
#include <SDL/SDL_syswm.h>

#ifdef __APPLE__
#include <OpenGL/OpenGL.h>
#endif

#ifdef linux
#include <sys/ioctl.h>
#include <sys/fcntl.h>
#endif
#ifdef AVG_ENABLE_XINERAMA
#include <X11/extensions/Xinerama.h>
#endif

#include <signal.h>
#include <iostream>
#include <sstream>
#include <math.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

using namespace std;

namespace avg {

void SDLDisplayEngine::initSDL()
{
#ifdef __APPLE__
    static bool bSDLInitialized = false;
    if (!bSDLInitialized) {
        CustomSDLMain();
        bSDLInitialized = true;
    }
#endif
#ifdef linux
    // Disable all other video drivers (DirectFB, libcaca, ...) to avoid confusing
    // error messages.
    SDL_putenv((char*)"SDL_VIDEODRIVER=x11");
#endif
    int err = SDL_InitSubSystem(SDL_INIT_VIDEO);
    if (err == -1) {
        throw Exception(AVG_ERR_VIDEO_INIT_FAILED, SDL_GetError());
    }
}

void SDLDisplayEngine::quitSDL()
{
#ifndef _WIN32
    SDL_QuitSubSystem(SDL_INIT_VIDEO);
#endif
}

SDLDisplayEngine::SDLDisplayEngine()
    : IInputDevice(EXTRACT_INPUTDEVICE_CLASSNAME(SDLDisplayEngine)),
      m_Size(0,0)
{
    initSDL();

    m_Gamma[0] = 1.0;
    m_Gamma[1] = 1.0;
    m_Gamma[2] = 1.0;
}

SDLDisplayEngine::~SDLDisplayEngine()
{
}

void SDLDisplayEngine::init(const DisplayParams& dp, GLConfig glConfig) 
{
    if (m_Gamma[0] != 1.0f || m_Gamma[1] != 1.0f || m_Gamma[2] != 1.0f) {
        internalSetGamma(1.0f, 1.0f, 1.0f);
    }

    m_pWindow = WindowPtr(new Window(dp, glConfig));
    Display::get()->getRefreshRate();

    setGamma(dp.m_Gamma[0], dp.m_Gamma[1], dp.m_Gamma[2]);
    showCursor(dp.m_bShowCursor);
    if (dp.m_Framerate == 0) {
        setVBlankRate(dp.m_VBRate);
    } else {
        setFramerate(dp.m_Framerate);
    }

    // SDL sets up a signal handler we really don't want.
    signal(SIGSEGV, SIG_DFL);
    VideoDecoder::logConfig();

    SDL_EnableUNICODE(1);
}

IntPoint SDLDisplayEngine::calcWindowSize(const DisplayParams& dp) const
{
    float aspectRatio = float(dp.m_Size.x)/float(dp.m_Size.y);
    IntPoint windowSize;
    if (dp.m_WindowSize == IntPoint(0, 0)) {
        windowSize = dp.m_Size;
    } else if (dp.m_WindowSize.x == 0) {
        windowSize.x = int(dp.m_WindowSize.y*aspectRatio);
        windowSize.y = dp.m_WindowSize.y;
    } else {
        windowSize.x = dp.m_WindowSize.x;
        windowSize.y = int(dp.m_WindowSize.x/aspectRatio);
    }
    AVG_ASSERT(windowSize.x != 0 && windowSize.y != 0);
    return windowSize;
}
        
void SDLDisplayEngine::setWindowTitle(const string& sTitle)
{
    SDL_WM_SetCaption(sTitle.c_str(), 0);
}

void SDLDisplayEngine::teardown()
{
    m_pWindow = WindowPtr();
}

void SDLDisplayEngine::setGamma(float red, float green, float blue)
{
    if (red > 0) {
        bool bOk = internalSetGamma(red, green, blue);
        m_Gamma[0] = red;
        m_Gamma[1] = green;
        m_Gamma[2] = blue;
        if (!bOk) {
            AVG_LOG_WARNING("Unable to set display gamma.");
        }
    }
}

void SDLDisplayEngine::setMousePos(const IntPoint& pos)
{
    SDL_WarpMouse(pos.x, pos.y);
}

int SDLDisplayEngine::getKeyModifierState() const
{
    return SDL_GetModState();
}

bool SDLDisplayEngine::internalSetGamma(float red, float green, float blue)
{
#ifdef __APPLE__
    // Workaround for broken SDL_SetGamma for libSDL 1.2.15 under Lion
    CGError err = CGSetDisplayTransferByFormula(kCGDirectMainDisplay, 0, 1, 1/red,
            0, 1, 1/green, 0, 1, 1/blue);
    return (err == CGDisplayNoErr);
#else
    int err = SDL_SetGamma(float(red), float(green), float(blue));
    return (err != -1);
#endif
}

void SDLDisplayEngine::showCursor(bool bShow)
{
#ifdef _WIN32
#define MAX_CORE_POINTERS   6
    // Hack to fix a pointer issue with fullscreen, SDL and touchscreens
    // Refer to Mantis bug #140
    for (int i = 0; i < MAX_CORE_POINTERS; ++i) {
        ShowCursor(bShow);
    }
#else
    if (bShow) {
        SDL_ShowCursor(SDL_ENABLE);
    } else {
        SDL_ShowCursor(SDL_DISABLE);
    }
#endif
}

BitmapPtr SDLDisplayEngine::screenshot(int buffer)
{
    return m_pWindow->screenshot(buffer);
}

vector<EventPtr> SDLDisplayEngine::pollEvents()
{
    return m_pWindow->pollEvents();
}
IntPoint SDLDisplayEngine::getSize()
{
    return m_Size;
}

const IntPoint SDLDisplayEngine::getWindowSize() const
{
    if (m_pWindow) {
        return m_pWindow->getSize();
    } else {
        return IntPoint(0,0);
    }
}

bool SDLDisplayEngine::isFullscreen() const
{
    return m_bIsFullscreen;
}

void SDLDisplayEngine::swapBuffers()
{
    m_pWindow->swapBuffers();
}

}
