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

#include "CGLContext.h"

#include "../base/Exception.h"
#include "../base/Logger.h"

#include <ApplicationServices/ApplicationServices.h>

#include <SDL2/SDL.h>

#include <iostream>


namespace avg {

using namespace std;
using namespace boost;

CGLContext::CGLContext(const GLConfig& glConfig, const IntPoint& windowSize, 
        const SDL_SysWMinfo* pSDLWMInfo)
    : GLContext(windowSize)
{
    if (pSDLWMInfo) {
        m_Context = CGLGetCurrentContext();
        AVG_ASSERT(m_Context);
        setCurrent();
    } else {
        CGLPixelFormatObj pixelFormatObj;
        GLint numPixelFormats;

        CGLPixelFormatAttribute attribs[] = {(CGLPixelFormatAttribute)NULL};
        CGLChoosePixelFormat(attribs, &pixelFormatObj, &numPixelFormats);

        CGLError err = CGLCreateContext(pixelFormatObj, 0, &m_Context);
        if (err) {
            cerr << CGLErrorString(err) << endl;
            AVG_ASSERT(false);
        }
        CGLDestroyPixelFormat(pixelFormatObj);
    }
    init(glConfig, !pSDLWMInfo);
}

CGLContext::~CGLContext()
{
    deleteObjects();
    if (m_Context && ownsContext()) {
        CGLSetCurrentContext(0);
        CGLDestroyContext(m_Context);
        m_Context = 0;
    }
}

void CGLContext::activate()
{
    CGLError err = CGLSetCurrentContext(m_Context);
    AVG_ASSERT(err == kCGLNoError);
    setCurrent();
}

void CGLContext::swapBuffers()
{
    CGLFlushDrawable(m_Context);
    GLContext::checkError("swapBuffers()");
}


}
