//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2012 Ulrich von Zadow
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

#include "BCMDisplay.h"

#include "../base/Exception.h"

#include <SDL/SDL.h>
#include <SDL/SDL_syswm.h>

#include <EGL/eglext_brcm.h>

#include <iostream>

using namespace std;

namespace avg {

IntPoint getX11WindowPosition(const SDL_SysWMinfo* pSDLWMInfo)
{
    int x, y;
    Window dummy;
    XWindowAttributes wAttribs;
    pSDLWMInfo->info.x11.lock_func();
    XGetWindowAttributes(pSDLWMInfo->info.x11.display, pSDLWMInfo->info.x11.window, &wAttribs);
    XTranslateCoordinates(pSDLWMInfo->info.x11.display, pSDLWMInfo->info.x11.window, wAttribs.root,
            0, 0, &x, &y, &dummy);
    pSDLWMInfo->info.x11.unlock_func();
    return IntPoint(x, y);
}

DISPMANX_DISPLAY_HANDLE_T getBCMDisplay(const SDL_SysWMinfo* pSDLWMInfo)
{
    bcm_host_init();

    DISPMANX_DISPLAY_HANDLE_T display = vc_dispmanx_display_open(DISPMANX_ID_MAIN_LCD);
    if (display == DISPMANX_NO_HANDLE) {
        throw Exception(AVG_ERR_VIDEO_GENERAL, "No BCM display available.");
    }
    return display;
}

EGL_DISPMANX_WINDOW_T* createChildWindow(const SDL_SysWMinfo* pSDLWMInfo,
        EGLNativeDisplayType bcmDisplay, const IntPoint& windowSize)
{
    // Create a child (yet toplevel) window with the required attributes to render into.
    IntPoint windowPos = getX11WindowPosition(pSDLWMInfo);
    VC_RECT_T dstRect;
    dstRect.x = windowPos.x;
    dstRect.y = windowPos.y;
    dstRect.width = windowSize.x;
    dstRect.height = windowSize.y;
    VC_RECT_T srcRect;
    srcRect.x = 0;
    srcRect.y = 0;
    srcRect.width = windowSize.x << 16;
    srcRect.height = windowSize.y << 16;
    // start display update
    DISPMANX_UPDATE_HANDLE_T update = vc_dispmanx_update_start(0);
    if (update == DISPMANX_NO_HANDLE) {
        throw Exception(AVG_ERR_VIDEO_GENERAL, "Failed to start BCM display update.");
    }
    // add element to display
    DISPMANX_ELEMENT_HANDLE_T element = vc_dispmanx_element_add(update,
            (DISPMANX_DISPLAY_HANDLE_T)bcmDisplay,
            0 /* layer */, &dstRect, DISPMANX_NO_HANDLE /* src */, &srcRect,
            (DISPMANX_PROTECTION_T) DISPMANX_PROTECTION_NONE,
            0 /* alpha */, 0 /* clamp */, DISPMANX_NO_ROTATE);
    if (element == DISPMANX_NO_HANDLE) {
        throw Exception(AVG_ERR_VIDEO_GENERAL, "Failed to add element to BCM display.");
    }
    // finish display update
    if (vc_dispmanx_update_submit_sync(update)) {
        throw Exception(AVG_ERR_VIDEO_GENERAL, "Failed to finish BCM display update.");
    }

    EGL_DISPMANX_WINDOW_T* pWin = new EGL_DISPMANX_WINDOW_T; // XXX: destroy
    pWin->element = element;
    pWin->width = windowSize.x;
    pWin->height = windowSize.y;
    return pWin;
}

EGLSurface createBCMPixmapSurface(EGLDisplay display, EGLConfig config)
{
    EGLint pixel_format = EGL_PIXEL_FORMAT_ARGB_8888_BRCM;
    EGLint rt;
    eglGetConfigAttrib(display, config, EGL_RENDERABLE_TYPE, &rt);

    if (rt & EGL_OPENGL_ES_BIT) {
        pixel_format |= EGL_PIXEL_FORMAT_RENDER_GLES_BRCM;
        pixel_format |= EGL_PIXEL_FORMAT_GLES_TEXTURE_BRCM;
    }
    if (rt & EGL_OPENGL_ES2_BIT) {
        pixel_format |= EGL_PIXEL_FORMAT_RENDER_GLES2_BRCM;
        pixel_format |= EGL_PIXEL_FORMAT_GLES2_TEXTURE_BRCM;
    }
    if (rt & EGL_OPENVG_BIT) {
        pixel_format |= EGL_PIXEL_FORMAT_RENDER_VG_BRCM;
        pixel_format |= EGL_PIXEL_FORMAT_VG_IMAGE_BRCM;
    }
    if (rt & EGL_OPENGL_BIT) {
        pixel_format |= EGL_PIXEL_FORMAT_RENDER_GL_BRCM;
    }

    EGLint pixmap[5];
    pixmap[0] = 0;
    pixmap[1] = 0;
    pixmap[2] = 8;
    pixmap[3] = 8;
    pixmap[4] = pixel_format;

    eglCreateGlobalImageBRCM(8, 8, pixel_format, 0, 8*4, pixmap);

    EGLSurface surface = eglCreatePixmapSurface(display, config, pixmap, 0);
    if ( surface == EGL_NO_SURFACE ) {
        cerr << "Unable to create EGL surface (eglError: " << eglGetError() << ")" << endl;
    }
    return surface;

}

}
