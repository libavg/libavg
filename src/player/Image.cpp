//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2006 Ulrich von Zadow
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

#include "Image.h"

#include "DisplayEngine.h"
#include "Player.h"
#include "ISurface.h"

#include "../graphics/Filtercolorize.h"
#include "../graphics/Filterfliprgb.h"

#include "../base/Logger.h"
#include "../base/ScopeTimer.h"
#include "../base/XMLHelper.h"

#include <Magick++.h>

#include <iostream>
#include <sstream>

using namespace std;

namespace avg {

Image::Image ()
    : m_Hue(-1),
      m_Saturation(-1)
{
    m_pBmp = BitmapPtr(new Bitmap(IntPoint(1,1), R8G8B8X8));
}

Image::Image (const xmlNodePtr xmlNode, Player * pPlayer)
    : RasterNode(xmlNode, pPlayer)
{
    m_href = getDefaultedStringAttr (xmlNode, "href", "");
    m_Hue = getDefaultedIntAttr (xmlNode, "hue", -1);
    m_Saturation = getDefaultedIntAttr (xmlNode, "saturation", -1);
    m_pBmp = BitmapPtr(new Bitmap(IntPoint(1,1), R8G8B8X8));
    load();
}

Image::~Image ()
{
}

void Image::setDisplayEngine(DisplayEngine * pEngine)
{
    RasterNode::setDisplayEngine(pEngine);

    setupSurface(&*m_pBmp);
    m_pBmp=BitmapPtr();
}

void Image::disconnect()
{
    if (m_pBmp != BitmapPtr()) {
        // Unload textures but keep bitmap in memory.
        ISurface * pSurface = getSurface();
        BitmapPtr pSurfaceBmp = pSurface->lockBmp();
        m_pBmp = BitmapPtr(new Bitmap(pSurfaceBmp->getSize(), pSurfaceBmp->getPixelFormat()));
        m_pBmp->copyPixels(*pSurfaceBmp);
        getSurface()->unlockBmps();
#ifdef __i386__
        // XXX Yuck
        if (!(getPlayer()->getDisplayEngine()->hasRGBOrdering())) {
            FilterFlipRGB().applyInPlace(m_pBmp);
        }
#endif
    }
    RasterNode::disconnect();
}

const std::string& Image::getHRef() const
{
    return m_href;
}

void Image::setHRef(const string& href)
{
    m_href = href;
    load();
    if (isDisplayAvailable()) {
        setupSurface(&*m_pBmp);
        m_pBmp=BitmapPtr();
    }
    DPoint Size = getPreferredMediaSize();
    setViewport(-32767, -32767, Size.x, Size.y);
}

void Image::setBitmap(const Bitmap * pBmp)
{
    // TODO: Add a unique bitmap identifier to the URI.
    m_href = "mem://";
    PixelFormat pf;
    pf = R8G8B8X8;
    if (pBmp->hasAlpha()) {
        pf = R8G8B8A8;
    }
    if (pBmp->getPixelFormat() == I8) {
        pf = I8;
    }
#ifdef __i386__
    if (!(getPlayer()->getDisplayEngine()->hasRGBOrdering())) {
        switch (pf) {
            case R8G8B8X8:
                pf = B8G8R8X8;
                break;
            case R8G8B8A8:
                pf = B8G8R8A8;
                break;
            default:
                break;
        }
    }
#endif
//    cerr << "pf: " << Bitmap::getPixelFormatString(pf) << endl;
    getSurface()->create(pBmp->getSize(), pf, true);
    BitmapPtr pSurfaceBmp = getSurface()->lockBmp();
    pSurfaceBmp->copyPixels(*pBmp);
    getSurface()->unlockBmps();
    getEngine()->surfaceChanged(getSurface());
    DPoint Size = getPreferredMediaSize();
    setViewport(-32767, -32767, Size.x, Size.y);
}

static ProfilingZone RenderProfilingZone("    Image::render");

void Image::render (const DRect& Rect)
{
    ScopeTimer Timer(RenderProfilingZone);
    if (m_href != "") {
        getEngine()->blt32(getSurface(), &getAbsViewport(), getEffectiveOpacity(), 
                getAngle(), getPivot(), getBlendMode());
    }
}

bool Image::obscures (const DRect& Rect, int Child) 
{
    PixelFormat pf = getSurface()->getPixelFormat();
    bool bHasAlpha = (pf == R8G8B8A8 || pf == B8G8R8A8);
    return (isActive() && getEffectiveOpacity() > 0.999
            && bHasAlpha && getVisibleRect().Contains(Rect));
}

string Image::getTypeStr ()
{
    return "Image";
}

DPoint Image::getPreferredMediaSize()
{
    if (isDisplayAvailable()) {
        return DPoint(getSurface()->getSize());
    } else {
        return DPoint(m_pBmp->getSize());
    }
}

void Image::load()
{
    m_Filename = m_href;
    if (m_Filename != "") {
        initFilename(getPlayer(), m_Filename);
        AVG_TRACE(Logger::PROFILE, "Loading " << m_Filename);
        try {
            m_pBmp = BitmapPtr(new Bitmap(m_Filename));
        } catch (Magick::Exception & ex) {
            AVG_TRACE(Logger::ERROR, ex.what());
        }
    } else {
        m_pBmp = BitmapPtr(new Bitmap(IntPoint(1,1), R8G8B8X8));
    }
    if (m_Saturation != -1) {
        FilterColorize(m_Hue, m_Saturation).applyInPlace(
                m_pBmp);
    }
}

void Image::setupSurface(const Bitmap * pBmp)
{
    PixelFormat pf;
    pf = R8G8B8X8;
    if (pBmp->hasAlpha()) {
        pf = R8G8B8A8;
    }
    bool bUsePBO = true;
#ifdef __APPLE__
    if (getSurface()->wouldTile(pBmp->getSize())) {
        bUsePBO = false;
    }
#endif
    getSurface()->create(pBmp->getSize(), pf, bUsePBO);
    BitmapPtr pSurfaceBmp = getSurface()->lockBmp();
    pSurfaceBmp->copyPixels(*pBmp);
#ifdef __i386__
    if (!(getPlayer()->getDisplayEngine()->hasRGBOrdering())) {
        FilterFlipRGB().applyInPlace(pSurfaceBmp);
    }
#endif
    getSurface()->unlockBmps();
    getEngine()->surfaceChanged(getSurface());
}

}
