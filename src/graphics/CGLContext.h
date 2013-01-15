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
#ifndef _CGLContext_H_
#define _CGLContext_H_
#include "../api.h"

#include "GLContext.h"

#include <OpenGL/OpenGL.h>
#undef check // Conflicts with Boost

#include <boost/shared_ptr.hpp>

struct SDL_SysWMinfo;

namespace avg {

class AVG_API CGLContext: public GLContext
{
public:
    CGLContext(const GLConfig& glConfig, const IntPoint& windowSize=IntPoint(0,0), 
            const SDL_SysWMinfo* pSDLWMInfo=0);
    virtual ~CGLContext();

    void activate();

    bool initVBlank(int rate);
    static float calcRefreshRate();

private:
    void initMacVBlank(int rate);

    CGLContextObj m_Context;
};

}
#endif


