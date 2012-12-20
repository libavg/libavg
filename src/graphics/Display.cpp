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

#include "Display.h"
#ifdef __linux__
#include "X11Display.h"
#endif
#include "Bitmap.h"

#include "../base/Exception.h"
#include "../base/Logger.h"

#include <SDL/SDL.h>
#include <SDL/SDL_syswm.h>

#ifdef _WIN32
#include <windows.h>
#endif

#ifdef __APPLE__
#include <ApplicationServices/ApplicationServices.h>
#endif

// Temp for queryRefreshRate
#ifdef __APPLE__
    #include "CGLContext.h"
#elif defined _WIN32
    #include "WGLContext.h"
#endif

#include <iostream>

using namespace std;

namespace avg {

DisplayPtr Display::get()
{
    static DisplayPtr s_pDisplay;
    if (!s_pDisplay) {
#ifdef __linux__
        s_pDisplay = DisplayPtr(new X11Display());
#else
        s_pDisplay = DisplayPtr(new Display());
#endif
        s_pDisplay->init();
    }
    return s_pDisplay;
}

Display::Display()
    : m_bAutoPPMM(true),
      m_RefreshRate(0.0f)
{
}

Display::~Display()
{
}

void Display::init()
{
    m_ScreenResolution = queryScreenResolution();
    m_PPMM = queryPPMM();
}

void Display::rereadScreenResolution()
{
    m_ScreenResolution = queryScreenResolution();
    if (m_bAutoPPMM) {
        m_PPMM = queryPPMM();
    }
}

IntPoint Display::getScreenResolution()
{
    return m_ScreenResolution;
}

float Display::getPixelsPerMM()
{
    return m_PPMM;
}

void Display::assumePixelsPerMM(float ppmm)
{
    if (ppmm != 0) {
        m_PPMM = ppmm;
        m_bAutoPPMM = false;
    }
}

glm::vec2 Display::getPhysicalScreenDimensions()
{
    glm::vec2 size;
    glm::vec2 screenRes = glm::vec2(getScreenResolution());
    size.x = screenRes.x/m_PPMM;
    size.y = screenRes.y/m_PPMM;
    return size;
}

float Display::queryPPMM()
{
#ifdef WIN32
    HDC hdc = CreateDC("DISPLAY", NULL, NULL, NULL);
    return GetDeviceCaps(hdc, LOGPIXELSX)/25.4f;
#else
#ifdef __APPLE__
    CGSize size = CGDisplayScreenSize(CGMainDisplayID());
    glm::vec2 displayMM(size.width, size.height);
    return m_ScreenResolution.x/displayMM.x;
#else
    AVG_ASSERT(false);
    return 0;
#endif
#endif
}

IntPoint Display::queryScreenResolution()
{
    const SDL_VideoInfo* pInfo = SDL_GetVideoInfo();
    return IntPoint(pInfo->current_w, pInfo->current_h);
}

float Display::getRefreshRate()
{
    if (m_RefreshRate == 0.0) {
        m_RefreshRate = queryRefreshRate();
        AVG_TRACE(Logger::CONFIG, "Vertical Refresh Rate: " << m_RefreshRate);
    }
    return m_RefreshRate;
}

float Display::queryRefreshRate()
{
#ifdef __APPLE__
    return CGLContext::calcRefreshRate();
#elif defined _WIN32
    return WGLContext::calcRefreshRate();
#else
    AVG_ASSERT(false);
    return 0.0;
#endif
}

}
