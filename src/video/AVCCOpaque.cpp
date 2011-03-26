//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2008 Ulrich von Zadow
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
#include "AVCCOpaque.h"


namespace avg{

AVCCOpaque::AVCCOpaque(VDPAU* vdpau, FrameAge* frameAge)
    : m_pVDPAU(vdpau),
      m_pFrameAge(frameAge)
{
}

FrameAge* AVCCOpaque::getFrameAge()
{
    return m_pFrameAge;
}

void AVCCOpaque::setFrameAge(FrameAge* age)
{
    m_pFrameAge = age;
}

VDPAU* AVCCOpaque::getVDPAU()
{
    return m_pVDPAU;
}

}

