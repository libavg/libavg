//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2008 Ulrich von Zadow
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

#ifndef _Win7TouchEventSource_H_
#define _Win7TouchEventSource_H_

#include "../api.h"
#include "MultitouchEventSource.h"

#undef WIN32_LEAN_AND_MEAN
#include <SDL/SDL_syswm.h>

namespace avg {

#ifdef SM_DIGITIZER
typedef bool (WINAPI* GTIIPROC)(HTOUCHINPUT, UINT, PTOUCHINPUT, int);
typedef bool (WINAPI* CTIHPROC)(HTOUCHINPUT);
#endif

class AVG_API Win7TouchEventSource: public MultitouchEventSource
{
public:
    Win7TouchEventSource();
    virtual ~Win7TouchEventSource();
    virtual void start();

private:
    static LRESULT APIENTRY touchWndSubclassProc(HWND hwnd, UINT uMsg,
        WPARAM wParam, LPARAM lParam);
    void onTouch(HWND hWnd, WPARAM wParam, LPARAM lParam);
    IntPoint calcClientAreaOffset() const;

    static Win7TouchEventSource* s_pInstance;

    HWND m_Hwnd;
    WNDPROC m_OldWndProc;
    int m_LastID;
    IntPoint m_ClientAreaOffset;

#ifdef SM_DIGITIZER
    GTIIPROC m_pGetTouchInputInfoProc;
    CTIHPROC m_pCloseTouchInputHandleProc;
#endif
};

typedef boost::shared_ptr<Win7TouchEventSource> Win7TouchEventSourcePtr;

}

#endif

