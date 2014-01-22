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

#include "WinDisplay.h"

#include <windows.h>

using namespace std;

namespace avg {



WinDisplay::WinDisplay()
{
}

WinDisplay::~WinDisplay()
{
}



float WinDisplay::queryPPMM()
{
    HDC hdc = CreateDC("DISPLAY", NULL, NULL, NULL);
    return GetDeviceCaps(hdc, LOGPIXELSX)/25.4f;
}

float WinDisplay::queryRefreshRate()
{
    float refreshRate;
    // This isn't correct for multi-monitor systems.
    HDC hDC = CreateDC("DISPLAY", NULL, NULL, NULL);
    refreshRate = float(GetDeviceCaps(hDC, VREFRESH));
    if (refreshRate < 2) {
        refreshRate = 60;
    }
    DeleteDC(hDC);
    return refreshRate;
}

}
