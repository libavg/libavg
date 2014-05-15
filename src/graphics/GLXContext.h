//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2014 Ulrich von Zadow
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
#ifndef _GLXContext_H_
#define _GLXContext_H_
#include "../api.h"

#include "GLContext.h"

#include "../base/Exception.h"

#include <boost/shared_ptr.hpp>

struct SDL_SysWMinfo;

namespace avg {

class AVG_API GLXContext: public GLContext
{
public:
    GLXContext(const IntPoint& windowSize=IntPoint(0,0));
    virtual ~GLXContext();

    void activate();
    bool initVBlank(int rate);
    bool useDepthBuffer() const;
    void swapBuffers();

    static bool haveARBCreateContext();

protected:
    XVisualInfo* createDetachedContext(::Display* pDisplay, GLConfig& glConfig);
    void setX11ErrorHandler();
    void resetX11ErrorHandler();
    void throwOnXError(int code=AVG_ERR_VIDEO_GENERAL);
    void initDrawable();

    ::GLXContext getGLXContext() const;
    ::Display* getDisplay() const;
    Colormap getColormap() const;

private:
    GLXFBConfig getFBConfig(::Display* pDisplay, GLConfig& glConfig);
    static int X11ErrorHandler(::Display * pDisplay, XErrorEvent * pErrEvent);
    void createGLXContext(GLConfig& glConfig, const IntPoint& windowSize, 
            const SDL_SysWMinfo* pSDLWMInfo);

    Colormap m_Colormap;
    ::Display* m_pDisplay;
    ::GLXContext m_Context;
    GLXDrawable m_Drawable;

    bool m_bVBlankActive;
};

}
#endif


