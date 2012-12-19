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

#include "DisplayInfo.h"
#include "Bitmap.h"

#include "../base/Exception.h"

#include <SDL/SDL.h>
#include <SDL/SDL_syswm.h>

#ifdef AVG_ENABLE_XINERAMA
#include <X11/extensions/Xinerama.h>
#endif

#ifdef _WIN32
#include <windows.h>
#endif

#ifdef __APPLE__
#include <ApplicationServices/ApplicationServices.h>
#endif

#include <iostream>

using namespace std;

namespace avg {

DisplayInfo::DisplayInfo()
{
    m_bAutoPPMM = true;
    m_ScreenResolution = queryScreenResolution();
    m_PPMM = queryPPMM();
}

DisplayInfo::~DisplayInfo()
{
}

void DisplayInfo::rereadScreenResolution()
{
    m_ScreenResolution = queryScreenResolution();
    if (m_bAutoPPMM) {
        m_PPMM = queryPPMM();
    }
}

IntPoint DisplayInfo::getScreenResolution()
{
    return m_ScreenResolution;
}

float DisplayInfo::getPixelsPerMM()
{
    return m_PPMM;
}

void DisplayInfo::assumePixelsPerMM(float ppmm)
{
    if (ppmm != 0) {
        m_PPMM = ppmm;
        m_bAutoPPMM = false;
    }
}

glm::vec2 DisplayInfo::getPhysicalScreenDimensions()
{
    glm::vec2 size;
    glm::vec2 screenRes = glm::vec2(getScreenResolution());
    size.x = screenRes.x/m_PPMM;
    size.y = screenRes.y/m_PPMM;
    return size;
}

float DisplayInfo::queryPPMM()
{
#ifdef WIN32
    HDC hdc = CreateDC("DISPLAY", NULL, NULL, NULL);
    return GetDeviceCaps(hdc, LOGPIXELSX)/25.4f;
#else
#ifdef linux
    Display * pDisplay = XOpenDisplay(0);
    glm::vec2 displayMM(DisplayWidthMM(pDisplay,0), DisplayHeightMM(pDisplay,0));
#elif defined __APPLE__
    CGSize size = CGDisplayScreenSize(CGMainDisplayID());
    glm::vec2 displayMM(size.width, size.height);
#endif
    // Non-Square pixels cause errors here. We'll fix that when it happens.
    return m_ScreenResolution.x/displayMM.x;
#endif
}

IntPoint DisplayInfo::queryScreenResolution()
{
    IntPoint size;
#ifdef __linux__
    bool bXinerama = false;
    Display * pDisplay = XOpenDisplay(0);
#ifdef AVG_ENABLE_XINERAMA
    int dummy1, dummy2;
    bXinerama = XineramaQueryExtension(pDisplay, &dummy1, &dummy2);
    if (bXinerama) {
        bXinerama = XineramaIsActive(pDisplay);
    }
    if (bXinerama) {
        int numHeads = 0;
        XineramaScreenInfo * pScreenInfo = XineramaQueryScreens(pDisplay, &numHeads);
        AVG_ASSERT(numHeads >= 1);
        /*
        cerr << "Num heads: " << numHeads << endl;
        for (int x=0; x<numHeads; ++x) {
            cout << "Head " << x+1 << ": " <<
                pScreenInfo[x].width << "x" << pScreenInfo[x].height << " at " <<
                pScreenInfo[x].x_org << "," << pScreenInfo[x].y_org << endl;
        }
        */
        size = IntPoint(pScreenInfo[0].width, pScreenInfo[0].height);  
        XFree(pScreenInfo);
    }
#endif
    if (!bXinerama) {
        Screen* pScreen = DefaultScreenOfDisplay(pDisplay);
        AVG_ASSERT(pScreen);
        size = IntPoint(pScreen->width, pScreen->height);
    }
    XCloseDisplay(pDisplay);
#else
    const SDL_VideoInfo* pInfo = SDL_GetVideoInfo();
    size = IntPoint(pInfo->current_w, pInfo->current_h);
#endif
    return size;
}

}
