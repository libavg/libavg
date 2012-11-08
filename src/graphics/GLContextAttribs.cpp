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

#include "GLContextAttribs.h"
#include "OGLHelper.h"

#include "../base/Exception.h"

namespace avg {

GLContextAttribs::GLContextAttribs()
{
    m_pAttributes = new int[50];
    m_NumAttributes = 0;
#ifdef AVG_ENABLE_EGL
    m_pAttributes[0] = EGL_NONE;
#else
    m_pAttributes[0] = 0;
#endif
}

GLContextAttribs::~GLContextAttribs()
{
    delete[] m_pAttributes;
}


void GLContextAttribs::append(int newAttr, int newAttrVal)
{
    AVG_ASSERT(m_NumAttributes < 48);

    m_pAttributes[m_NumAttributes++] = newAttr;
    if (newAttrVal != -1) {
        m_pAttributes[m_NumAttributes++] = newAttrVal;
    }
#ifdef AVG_ENABLE_EGL
    m_pAttributes[m_NumAttributes] = EGL_NONE;
#else
    m_pAttributes[m_NumAttributes] = 0;
#endif
}

const int* GLContextAttribs::get() const
{
    return m_pAttributes;
}

}

