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

#include "BCMDisplay.h"

#include "../base/Exception.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>

#include <EGL/eglext_brcm.h>

#include <iostream>

using namespace std;

namespace avg {

BCMDisplay::BCMDisplay()
{
}

BCMDisplay::~BCMDisplay()
{
}

float BCMDisplay::queryPPMM()
{
    // TODO
    return 40;
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
