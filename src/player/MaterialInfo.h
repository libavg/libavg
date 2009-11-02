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

#ifndef _MaterialInfo_H_
#define _MaterialInfo_H_

#include "../api.h"

#include "../base/Point.h"

namespace avg {

class AVG_API MaterialInfo {
public:
    MaterialInfo(int texWrapSMode, int texWrapTMode, bool bUseMipmaps);

    void setMask(bool bHasMask, bool bIsWords = false, const DPoint& pos = DPoint(0,0), 
            const DPoint& size = DPoint(0,0), const DPoint& mediaSize = DPoint(0,0));
    bool getHasMask() const;
    const DPoint& getMaskPos() const;
    const DPoint& getMaskSize() const;

    int getTexWrapSMode() const;
    int getTexWrapTMode() const;

    void setUseMipmaps(bool bUseMipmaps);
    bool getUseMipmaps() const;

private:
    int m_TexWrapSMode;
    int m_TexWrapTMode;
    bool m_bUseMipmaps;

    bool m_bHasMask;
    DPoint m_MaskPos;
    DPoint m_MaskSize;
};

}

#endif


