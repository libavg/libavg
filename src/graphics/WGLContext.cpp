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

#include "WGLContext.h"

#include "../base/Exception.h"
#include "../base/Logger.h"

#include <SDL/SDL.h>
#undef WIN32_LEAN_AND_MEAN
#include <SDL/SDL_syswm.h>

#include <iostream>


namespace avg {

using namespace std;
using namespace boost;

LONG WINAPI imagingWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{ 
    return DefWindowProc(hwnd, msg, wParam, lParam); 
} 

void registerWindowClass()
{
    static char * pClassName;
    if (pClassName) {
        return;
    }
    pClassName = "GL";

    HINSTANCE hInstance = GetModuleHandle(NULL);
    WNDCLASS  wc;
    memset(&wc, 0, sizeof(WNDCLASS));
    wc.style = CS_OWNDC;
    wc.lpfnWndProc = (WNDPROC)imagingWindowProc;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);
    wc.hCursor = LoadCursor(hInstance, IDC_ARROW);
    wc.hbrBackground = NULL;
    wc.lpszMenuName = NULL;
    wc.lpszClassName = pClassName;

    BOOL bOK = RegisterClass(&wc);
    AVG_ASSERT(bOK);
}

WGLContext::WGLContext(const GLConfig& glConfig, const IntPoint& windowSize, 
        const SDL_SysWMinfo* pSDLWMInfo)
    : GLContext(glConfig, windowSize, pSDLWMInfo)
{
        bool bOwnsContext;
    if (pSDLWMInfo) {
        m_hDC = wglGetCurrentDC();
        m_Context = wglGetCurrentContext();
        setCurrent();
        bOwnsContext = false;
    } else {
        registerWindowClass();
        m_hwnd = CreateWindow("GL", "GL",
                WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
                0, 0, 500, 300, 0, 0, GetModuleHandle(NULL), 0);
        checkWinError(m_hwnd != 0, "CreateWindow");

        m_hDC = GetDC(m_hwnd);
        checkWinError(m_hDC != 0, "GetDC");

        PIXELFORMATDESCRIPTOR pfd;
        ZeroMemory(&pfd, sizeof(pfd));
        pfd.nSize = sizeof(pfd);
        pfd.nVersion = 1;
        pfd.dwFlags = PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW;
        pfd.iPixelType = PFD_TYPE_RGBA;
        pfd.cColorBits = 32;
        pfd.cDepthBits = 32;
        pfd.iLayerType = PFD_MAIN_PLANE;

        int iFormat = ChoosePixelFormat(m_hDC, &pfd);
        checkWinError(iFormat != 0, "ChoosePixelFormat");
        SetPixelFormat(m_hDC, iFormat, &pfd);
        m_Context = wglCreateContext(m_hDC);
        checkWinError(m_Context != 0, "wglCreateContext");
        bOwnsContext = true;
    }

    init(bOwnsContext);
}

WGLContext::~WGLContext()
{
    deleteObjects();
    if (m_Context && ownsContext()) {
        wglDeleteContext(m_Context);
        DeleteDC(m_hDC);
        DestroyWindow(m_hwnd);
    }
}

void WGLContext::activate()
{
    BOOL bOk = wglMakeCurrent(m_hDC, m_Context);
    checkWinError(bOk, "wglMakeCurrent");
    setCurrent();
}

bool WGLContext::initVBlank(int rate) 
{
    static bool s_bVBlankActive = false;
    if (rate > 0) {
        if (!queryOGLExtension("WGL_EXT_swap_control")
                && !queryWGLExtension("WGL_EXT_swap_control")) {
            AVG_LOG_WARNING(
                    "Windows VBlank setup failed: OpenGL Extension not supported.");
            s_bVBlankActive = false;
            return false;
        }
        glproc::SwapIntervalEXT(rate);
        s_bVBlankActive = true;
        return true;
    } else {
        if (s_bVBlankActive) {
            glproc::SwapIntervalEXT(0);
            s_bVBlankActive = false;
        }
        return false;
    }
}

bool WGLContext::queryWGLExtension(const char *extName)
{
    if (glproc::GetExtensionsStringARB == NULL) {
        return false;
    }

    char *p;
    size_t extNameLen = strlen(extName);

    p = (char *)glproc::GetExtensionsStringARB(m_hDC);
    AVG_ASSERT(p != 0);
    char * end = p + strlen(p);

    while (p < end) {
        size_t n = strcspn(p, " ");
        if ((extNameLen == n) && (strncmp(extName, p, n) == 0)) {
            return true;
        }
        p += (n + 1);
    }
    return false;
}

void WGLContext::checkWinError(BOOL bOK, const string& sWhere) 
{
    if (!bOK) {
        char szErr[512];
        FormatMessage((FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM),
                0, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                szErr, 512, 0);
        AVG_LOG_ERROR(sWhere+":"+szErr);
        AVG_ASSERT(false);
    }
}

}
