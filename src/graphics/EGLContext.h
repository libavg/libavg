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
#ifndef _EGLContext_H_
#define _EGLContext_H_

#include "../api.h"

#include <EGL/egl.h>
#include "GLContext.h"

struct SDL_SysWMinfo;

namespace avg {
    
class AVG_API EGLContext: public GLContext
{
public:
    EGLContext(const GLConfig& glConfig, const IntPoint& windowSize=IntPoint(0,0),
            const SDL_SysWMinfo* pSDLWMInfo=0);
    virtual ~EGLContext();

    void activate();
    bool initVBlank(int rate);
    void swapBuffers();
    static float calcRefreshRate();

private:
    void createEGLContext(const GLConfig& glConfig, const IntPoint& windowSize,
            const SDL_SysWMinfo* pSDLWMInfo);
    void checkEGLError(bool bError, const std::string& sMsg);

    void dumpEGLConfig() const;
    void dumpEGLConfigAttrib(EGLint attrib, const std::string& name) const;

    EGLNativeDisplayType m_xDisplay;
    EGLDisplay m_Display;
    EGLConfig m_Config;
    ::EGLContext m_Context;
    EGLSurface m_Surface;
};

}

#endif
