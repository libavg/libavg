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

#include "Win7TouchEventSource.h"
#include "TouchEvent.h"
#include "Player.h"
#include "AVGNode.h"
#include "TouchStatus.h"

#include "../base/Logger.h"
#include "../base/ObjectCounter.h"
#include "../base/Exception.h"

using namespace std;

namespace avg {

Win7TouchEventSource* Win7TouchEventSource::s_pInstance(0);

Win7TouchEventSource::Win7TouchEventSource()
    : m_LastID(0)
{
    s_pInstance = this;
}

Win7TouchEventSource::~Win7TouchEventSource()
{
    SetWindowLong(m_Hwnd, GWL_WNDPROC, (LONG)m_OldWndProc);
    s_pInstance = 0;
}

void Win7TouchEventSource::start()
{
#ifdef SM_DIGITIZER_
    int multitouchCaps = GetSystemMetrics(SM_DIGITIZER);
    if (multitouchCaps & NID_MULTI_INPUT) {
        AVG_TRACE(Logger::CONFIG, "Enabled Windows 7 Touch driver.");

        MultitouchEventSource::start();
        SDL_SysWMinfo info;
        SDL_VERSION(&info.version);
        int err = SDL_GetWMInfo(&info);
        AVG_ASSERT(err == 1);
        m_Hwnd = info.window;
        RegisterTouchWindow(m_Hwnd, 0);

        m_OldWndProc = (WNDPROC)SetWindowLong(m_Hwnd, GWL_WNDPROC, (LONG)touchWndSubclassProc);
    } else {
        throw Exception(AVG_ERR_UNSUPPORTED, "No multitouch device connected.");
    }
#else
    throw Exception(AVG_ERR_UNSUPPORTED, 
            "Windows multitouch not supported by this version of libavg.");
#endif
}

LRESULT APIENTRY Win7TouchEventSource::touchWndSubclassProc(HWND hwnd, UINT uMsg,
        WPARAM wParam, LPARAM lParam)
{
#ifdef SM_DIGITIZER_
    Win7TouchEventSource * pThis = Win7TouchEventSource::s_pInstance;
    if (uMsg == WM_TOUCH) {
        cerr << "WM_TOUCH" << endl;
        pThis->onTouch(hwnd, wParam, lParam);
    }

    return CallWindowProc(pThis->m_OldWndProc, hwnd, uMsg, wParam, lParam); 
#else
    return 0;
#endif
} 

void Win7TouchEventSource::onTouch(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
#ifdef SM_DIGITIZER_
    unsigned numInputs = LOWORD(wParam);
    PTOUCHINPUT pInputs = new TOUCHINPUT[numInputs];
    BOOL bOk = GetTouchInputInfo((HTOUCHINPUT)lParam, numInputs, pInputs, 
            sizeof(TOUCHINPUT));
    AVG_ASSERT(bOk);
    for (unsigned i = 0; i < numInputs; i++){
        TOUCHINPUT ti = pInputs[i];
            //do something with each touch input entry
    }            
    delete [] pInputs;
    CloseTouchInputHandle((HTOUCHINPUT)lParam);
#endif
}


}
