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

#include "MaterialInfo.h"

#include <iostream>

using namespace std;

namespace avg {

MaterialInfo::MaterialInfo(int texWrapSMode, int texWrapTMode, bool bUseMipmaps)
    : m_TexWrapSMode(texWrapSMode),
      m_TexWrapTMode(texWrapTMode),
      m_bUseMipmaps(bUseMipmaps),
      m_bHasMask(false)
{}

void MaterialInfo::setMask(bool bHasMask, bool bIsWords, const DPoint& pos, 
        const DPoint& size, const DPoint& mediaSize)
{
    m_bHasMask = bHasMask;
    if (m_bHasMask) {
        if (size == DPoint(0,0)) {
            m_MaskSize = DPoint(1,1);
        } else {
            m_MaskSize = DPoint(size.x/mediaSize.x, size.y/mediaSize.y);
        }
        if (size == DPoint(0,0) || !bIsWords) {
            m_MaskPos = DPoint(pos.x/mediaSize.x, pos.y/mediaSize.y);
        } else {
            m_MaskPos = DPoint(pos.x/size.x, pos.y/size.y);
        }
    }
}

bool MaterialInfo::getHasMask() const
{
    return m_bHasMask;
}

const DPoint& MaterialInfo::getMaskPos() const
{
    return m_MaskPos;
}

const DPoint& MaterialInfo::getMaskSize() const
{
    return m_MaskSize;
}

int MaterialInfo::getTexWrapSMode() const
{
    return m_TexWrapSMode;
}

int MaterialInfo::getTexWrapTMode() const
{
    return m_TexWrapTMode;
}

void MaterialInfo::setUseMipmaps(bool bUseMipmaps)
{
    m_bUseMipmaps = bUseMipmaps;
}

bool MaterialInfo::getUseMipmaps() const
{
    return m_bUseMipmaps;
}

}
