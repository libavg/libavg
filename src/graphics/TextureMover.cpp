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

#include "TextureMover.h"
#include "PBO.h"
#include "BmpTextureMover.h"
#include "GLContext.h"

#include "../base/Exception.h"

#include <iostream>
#include <cstring>

using namespace std;
using namespace boost;

namespace avg {

TextureMoverPtr TextureMover::create(OGLMemoryMode memoryMode, IntPoint size, 
        PixelFormat pf, unsigned usage)
{
    switch (memoryMode) {
#ifndef AVG_ENABLE_EGL
        case MM_PBO:
            return TextureMoverPtr(new PBO(size, pf, usage));
#endif
        case MM_OGL:
            return TextureMoverPtr(new BmpTextureMover(size, pf));
        default:
            AVG_ASSERT(false);
            return TextureMoverPtr();
    }
}

TextureMoverPtr TextureMover::create(IntPoint size, PixelFormat pf, unsigned usage)
{
    OGLMemoryMode memMode = GLContext::getCurrent()->getMemoryMode();
    return create(memMode, size, pf, usage);
}

TextureMover::TextureMover(const IntPoint& size, PixelFormat pf)
    : m_Size(size),
      m_pf(pf)
{
}

TextureMover::~TextureMover()
{
}

PixelFormat TextureMover::getPF() const
{
    return m_pf;
}

const IntPoint& TextureMover::getSize() const
{
    return m_Size;
}

}
