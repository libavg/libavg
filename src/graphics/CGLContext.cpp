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

#include "CGLContext.h"

#include "../base/Exception.h"
#include "../base/Logger.h"

#ifdef __APPLE__
#include <ApplicationServices/ApplicationServices.h>
#endif

#include <SDL/SDL.h>
#include <SDL/SDL_syswm.h>


#include <iostream>


namespace avg {

using namespace std;
using namespace boost;

CGLContext::CGLContext(const GLConfig& glConfig, const IntPoint& windowSize, 
        const SDL_SysWMinfo* pSDLWMInfo)
    : GLContext(glConfig, windowSize, pSDLWMInfo)
{
    if (pSDLWMInfo) {
        m_Context = CGLGetCurrentContext();
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
    init(!pSDLWMInfo);
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

bool CGLContext::initVBlank(int rate) 
{
    if (rate > 0) {
        initMacVBlank(rate);
        return true;
    } else {
        initMacVBlank(0);
        return false;
    }
}

void CGLContext::initMacVBlank(int rate)
{
    CGLContextObj context = CGLGetCurrentContext();
    AVG_ASSERT (context);
#if MAC_OS_X_VERSION_10_5
    GLint l = rate;
#else
    long l = rate;
#endif
    if (rate > 1) {
        AVG_TRACE(Logger::category::CONFIG, Logger::severity::WARNING,
                "VBlank rate set to " << rate 
                << " but Mac OS X only supports 1. Assuming 1.");
        l = 1;
    }
    CGLError err = CGLSetParameter(context, kCGLCPSwapInterval, &l);
    AVG_ASSERT(!err);
}

}
