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

#include "VAAPIHelper.h"

#include "../base/Exception.h"
#include "../base/ConfigMgr.h"

#include "../graphics/Bitmap.h"
#include "../graphics/X11Display.h"

#include <va/va_glx.h>

using namespace std;

namespace avg {

VADisplay getVAAPIDisplay()
{
    static bool bIsInitialized = false;
    static VADisplay vaDisplay = 0;
    if (!bIsInitialized) {
        bIsInitialized = true;
        ::Display* pDisplay = getX11Display(0);
        vaDisplay = vaGetDisplayGLX(pDisplay);

        int majorVer;
        int minorVer;
        VAStatus status = vaInitialize(vaDisplay, &majorVer, &minorVer);
        if (status != VA_STATUS_SUCCESS) {
            vaDisplay = 0;
            throw Exception(AVG_ERR_VIDEO_INIT_FAILED, vaErrorStr(status));
        }
    }
    return vaDisplay;
}

}
