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

#include "Display.h"
#ifdef __linux__
    #ifdef AVG_ENABLE_RPI
    #include "BCMDisplay.h"
    #else
    #include "X11Display.h"
    #endif
#endif
#ifdef __APPLE__
#include "AppleDisplay.h"
#endif
#ifdef _WIN32
#include "WinDisplay.h"
#endif
#include "Bitmap.h"

#include "../base/Logger.h"

#include <iostream>

using namespace std;

namespace avg {

DisplayPtr Display::s_pInstance = DisplayPtr();

DisplayPtr Display::get()
{
    if (!s_pInstance) {
#ifdef __linux__
    #ifdef AVG_ENABLE_RPI
        s_pInstance = DisplayPtr(new BCMDisplay());
    #else
        s_pInstance = DisplayPtr(new X11Display());
    #endif
#elif defined __APPLE__
        s_pInstance = DisplayPtr(new AppleDisplay());
#elif defined _WIN32
        s_pInstance = DisplayPtr(new WinDisplay());
#else
        AVG_ASSERT(false);
#endif
        s_pInstance->init();
    }
    return s_pInstance;
}

bool Display::isInitialized()
{
    return (s_pInstance != DisplayPtr());
}

Display::Display()
    : m_bAutoPPMM(true),
      m_RefreshRate(0)
{
}

Display::~Display()
{
}

void Display::init()
{
    queryScreenResolution();
    m_PPMM = queryPPMM();
}

void Display::rereadScreenResolution()
{
    queryScreenResolution();
    if (m_bAutoPPMM) {
        m_PPMM = queryPPMM();
    }
}

IntPoint Display::getScreenResolution()
{
    return IntPoint(m_DisplayMode.w, m_DisplayMode.h);
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

float Display::getRefreshRate()
{
    if (m_RefreshRate == 0) {
        queryScreenResolution();
        m_RefreshRate = m_DisplayMode.refresh_rate;
        if (m_RefreshRate == 0) {
            AVG_TRACE(Logger::category::CONFIG, Logger::severity::INFO,
                    "Could not get current refresh rate. Defaulting to 60 Hz");
            m_RefreshRate = 60;
        } else {
            AVG_TRACE(Logger::category::CONFIG, Logger::severity::INFO,
                    "Vertical Refresh Rate: " << m_RefreshRate);
        }
    }
    return (float)m_RefreshRate;
}

void Display::queryScreenResolution()
{
    int err = SDL_GetCurrentDisplayMode(0, &m_DisplayMode);
    AVG_ASSERT(err == 0);
}

}
