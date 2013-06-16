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

#include "BitmapLoader.h"

#include "PixelFormat.h"
#include "Filterfliprgb.h"

#include "../base/Exception.h"
#include "../base/ScopeTimer.h"

#include <gdk-pixbuf/gdk-pixbuf.h>
#include <iostream>

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
    AVG_ASSERT(s_pBitmapLoader != 0);
    GError* pError = 0;
    GdkPixbuf* pPixBuf;
    {
        ScopeTimer timer(GDKPixbufProfilingZone);
        pPixBuf = gdk_pixbuf_new_from_file(sFName.c_str(), &pError);
    }
    if (!pPixBuf) {
        string sErr = pError->message;
        g_error_free(pError);
        throw Exception(AVG_ERR_FILEIO, sErr);
    }
    IntPoint size = IntPoint(gdk_pixbuf_get_width(pPixBuf), 
            gdk_pixbuf_get_height(pPixBuf));
    
    PixelFormat srcPF;
    if (gdk_pixbuf_get_has_alpha(pPixBuf)) {
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

        int stride = gdk_pixbuf_get_rowstride(pPixBuf);
        guchar* pSrc = gdk_pixbuf_get_pixels(pPixBuf);
        BitmapPtr pSrcBmp(new Bitmap(size, srcPF, pSrc, stride, false));
        {
            ScopeTimer timer(RGBFlipProfilingZone);
            if (pixelFormatIsBlueFirst(pf) != pixelFormatIsBlueFirst(srcPF)) {
                FilterFlipRGB().applyInPlace(pSrcBmp);
            }
        }
        pBmp->copyPixels(*pSrcBmp);
    }
    g_object_unref(pPixBuf);
    return pBmp;
}

BitmapPtr loadBitmap(const UTF8String& sFName, PixelFormat pf)
{
    return BitmapLoader::get()->load(sFName, pf);
}

}

