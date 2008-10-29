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
#ifndef _OGLImagingContext_H_
#define _OGLImagingContext_H_
#include "../base/Point.h"

#include "OGLHelper.h"

#ifdef __APPLE__
#include <AGL/agl.h>
#else
#ifdef linux
#include <GL/glx.h>
#else
#ifdef _WIN32
#include <gl/gl.h>
#include <gl/glu.h>
#endif
#endif
#endif

namespace avg {

class OGLImagingContext {
public:
    OGLImagingContext(const IntPoint & size);
    virtual ~OGLImagingContext();

    void activate();
    void setSize(const IntPoint & size);

    bool isSupported();

private:
    IntPoint m_Size;
#ifdef __APPLE__
    AGLContext m_Context;
#else
#ifdef linux
    GLXContext m_Context;
#else
#ifdef _WIN32
    HWND m_hwnd;
    HDC m_hDC;
    HBITMAP m_hBitmap;
    void *m_pBits;
	HGLRC m_Context;
#endif
#endif
#endif
};
}
#endif

