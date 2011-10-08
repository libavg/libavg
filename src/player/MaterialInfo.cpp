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

#include "MaterialInfo.h"

#include "../base/Exception.h"

#include <iostream>

using namespace std;

namespace avg {

MaterialInfo::MaterialInfo(int wrapSMode, int wrapTMode, bool bUseMipmaps)
    : m_WrapSMode(wrapSMode),
      m_WrapTMode(wrapTMode),
      m_bUseMipmaps(bUseMipmaps)
{}

int MaterialInfo::getWrapSMode() const
{
    return m_WrapSMode;
}

int MaterialInfo::getWrapTMode() const
{
    return m_WrapTMode;
}

bool MaterialInfo::getUseMipmaps() const
{
    return m_bUseMipmaps;
}

}
