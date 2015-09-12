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
#include "../base/Rect.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <boost/shared_ptr.hpp>

struct SDL_SysWMinfo;

namespace avg {

class AVG_API GLXContext: public GLContext
{
public:
    GLXContext(const GLConfig& glConfig, const IntPoint& windowSize=IntPoint(0,0), 
            const SDL_SysWMinfo* pSDLWMInfo=0);
    GLXContext(const GLConfig& glConfig, const std::string& sDisplay,
            const IntRect& windowDimensions, bool bHasWindowFrame);
    virtual ~GLXContext();

    void activate();
    bool useDepthBuffer() const;
    void swapBuffers();

    static bool haveARBCreateContext();
    static bool isGLESSupported();

private:
    void createGLXContext(GLConfig& glConfig, const IntPoint& windowSize, 
            const SDL_SysWMinfo* pSDLWMInfo);
    void createContextAndWindow(GLConfig& glConfig, const std::string& sDisplay,
        const IntRect& windowDimensions, bool bHasWindowFrame);
    XVisualInfo* createDetachedContext(::Display* pDisplay, GLConfig& glConfig);
    void setX11ErrorHandler();
    void resetX11ErrorHandler();
    void throwOnXError(int code=AVG_ERR_VIDEO_GENERAL);

    ::Display* getDisplay() const;
    Colormap getColormap() const;

    GLXFBConfig getFBConfig(GLConfig& glConfig);
    static int X11ErrorHandler(::Display * pDisplay, XErrorEvent * pErrEvent);

    Colormap m_Colormap;
    ::Display* m_pDisplay;
    bool m_bOwnsDisplay;
    ::GLXContext m_Context;
    GLXDrawable m_Drawable;

    ::Window m_Window; // Only used if window not created by libSDL.

    bool m_bVBlankActive;
};

}
#endif


