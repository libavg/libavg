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

#include "WrapMode.h"
#include "OGLHelper.h"

namespace avg {
    
WrapMode::WrapMode()
    : m_S(GL_CLAMP_TO_EDGE),
      m_T(GL_CLAMP_TO_EDGE)
{
}

WrapMode::WrapMode(int s, int t)
    : m_S(s),
      m_T(t)
{}

int WrapMode::getS() const
{
    return m_S;
}

int WrapMode::getT() const
{
    return m_T;
}

}
