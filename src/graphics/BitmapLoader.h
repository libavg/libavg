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

#ifndef _BitmapLoader_H_ 
#define _BitmapLoader_H_

#include "../api.h"
#include "../avgconfigwrapper.h"

#include "Bitmap.h"
#include "PixelFormat.h"

#include <string>

namespace avg {

class AVG_API BitmapLoader {
public:
    static void init(bool bBlueFirst);
    static BitmapLoader* get();
    bool isBlueFirst() const;
    PixelFormat getDefaultPixelFormat(bool bAlpha);
    BitmapPtr load(const UTF8String& sFName, PixelFormat pf=NO_PIXELFORMAT) const;

private:
    BitmapLoader(bool bBlueFirst);
    virtual ~BitmapLoader();

    bool m_bBlueFirst;
    static BitmapLoader * s_pBitmapLoader;
};

BitmapPtr AVG_API loadBitmap(const UTF8String& sFName, PixelFormat pf=NO_PIXELFORMAT);

}

#endif
