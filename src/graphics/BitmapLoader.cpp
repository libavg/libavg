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

#include "BitmapLoader.h"

#include "PixelFormat.h"
#include "Filterfliprgb.h"

#include "../base/Exception.h"
#include "../base/ScopeTimer.h"

#include <iostream>
#include <memory>

using namespace std;
using namespace boost;

namespace avg {

BitmapLoader * BitmapLoader::s_pBitmapLoader = 0;
    
void BitmapLoader::init(bool bBlueFirst) 
{
    //    cerr << "BitmapLoader::init(" << bBlueFirst << ")" << endl;
    if (s_pBitmapLoader != 0) {
        delete s_pBitmapLoader;
    }
    s_pBitmapLoader = new BitmapLoader(bBlueFirst);
}

BitmapLoader* BitmapLoader::get() 
{
    AVG_ASSERT(s_pBitmapLoader != 0);
    return s_pBitmapLoader;
}

BitmapLoader::BitmapLoader(bool bBlueFirst)
    : m_bBlueFirst(bBlueFirst)
{
}

BitmapLoader::~BitmapLoader() 
{
}

bool BitmapLoader::isBlueFirst() const
{
    return m_bBlueFirst;
}

PixelFormat BitmapLoader::getDefaultPixelFormat(bool bAlpha)
{
    if (bAlpha) {
        if (m_bBlueFirst) {
            return B8G8R8A8;
        } else {
            return R8G8B8A8;
        }
    } else {
        if (m_bBlueFirst) {
            return B8G8R8X8;
        } else {
            return R8G8B8X8;
        }
    } 
}

static ProfilingZoneID GDKPixbufProfilingZone("gdk_pixbuf load", true);
static ProfilingZoneID ConvertProfilingZone("Format conversion", true);
static ProfilingZoneID RGBFlipProfilingZone("RGB<->BGR flip", true);

BitmapPtr BitmapLoader::load(const UTF8String& sFName, PixelFormat pf) const
{
    IntPoint size = IntPoint(128, 128);
    
    PixelFormat srcPF;
    bool src_has_alpha = false;
    if (src_has_alpha) {
        srcPF = R8G8B8A8;
    } else {
        srcPF = R8G8B8;
    }
    if (pf == NO_PIXELFORMAT) {
        if (m_bBlueFirst) {
            if (srcPF == R8G8B8A8) {
                pf = B8G8R8A8;
            } else if (srcPF == R8G8B8) {
                pf = B8G8R8X8;
            }
        } else {
            if (srcPF == R8G8B8A8) {
                pf = R8G8B8A8;
            } else if (srcPF == R8G8B8) {
                pf = R8G8B8X8;
            }
        }
    }

    BitmapPtr pBmp(new Bitmap(size, pf, sFName));
    {
        ScopeTimer timer(ConvertProfilingZone);

        int stride = 4 * size.x;
        unique_ptr<uint8_t[]> src(new uint8_t[4 * size.x * size.y]);
        BitmapPtr pSrcBmp(new Bitmap(size, srcPF, src.get(), stride, false));
        {
            ScopeTimer timer(RGBFlipProfilingZone);
            if (pixelFormatIsBlueFirst(pf) != pixelFormatIsBlueFirst(srcPF)) {
                FilterFlipRGB().applyInPlace(pSrcBmp);
            }
        }
        pBmp->copyPixels(*pSrcBmp);
    }
    return pBmp;
}

BitmapPtr loadBitmap(const UTF8String& sFName, PixelFormat pf)
{
    return BitmapLoader::get()->load(sFName, pf);
}

}

