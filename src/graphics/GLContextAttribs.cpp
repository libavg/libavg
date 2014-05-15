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

#include "GLContextAttribs.h"
#include "OGLHelper.h"

#include "../base/Exception.h"

#ifdef AVG_ENABLE_EGL
    #define ATTR_NONE EGL_NONE
#else
    #define ATTR_NONE 0
#endif


namespace avg {

GLContextAttribs::GLContextAttribs()
{
    m_pAttributes = new int[50];
    m_NumAttributes = 0;
    m_pAttributes[0] = ATTR_NONE;
}

GLContextAttribs::~GLContextAttribs()
{
    delete[] m_pAttributes;
}

GLContextAttribs& GLContextAttribs::operator=(const GLContextAttribs& rhs){
    m_NumAttributes = rhs.m_NumAttributes;
    m_pAttributes = new int[50];
    for(int i = 0; i<m_NumAttributes; i++){
        m_pAttributes[i] = rhs.m_pAttributes[i];
    }
    m_pAttributes[m_NumAttributes] = ATTR_NONE;
    return *this;
}

void GLContextAttribs::append(int newAttr, int newAttrVal)
{
    AVG_ASSERT(m_NumAttributes < 48);

    m_pAttributes[m_NumAttributes++] = newAttr;
    if (newAttrVal != -1) {
        m_pAttributes[m_NumAttributes++] = newAttrVal;
    }
    m_pAttributes[m_NumAttributes] = ATTR_NONE;
}

const int* GLContextAttribs::get() const
{
    return m_pAttributes;
}

}

