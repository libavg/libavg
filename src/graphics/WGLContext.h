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
#ifndef _WGLContext_H_
#define _WGLContext_H_
#include "../api.h"

#include "GLContext.h"

#include <boost/shared_ptr.hpp>

struct SDL_SysWMinfo;

namespace avg {

class AVG_API WGLContext: public GLContext
{
public:
    WGLContext(const GLConfig& glConfig, const IntPoint& windowSize=IntPoint(0,0), 
            const SDL_SysWMinfo* pSDLWMInfo=0);
    virtual ~WGLContext();

    void activate();

    bool initVBlank(int rate);

private:
    bool queryWGLExtension(const char* extName);
    void checkWinError(BOOL bOK, const std::string& sWhere);

    HWND m_hwnd;
    HDC m_hDC;
    HGLRC m_Context;
};

}
#endif


