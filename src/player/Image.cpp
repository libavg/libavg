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
    m_pBmp = BitmapPtr(new Bitmap(IntPoint(1,1), R8G8B8));
}

Image::Image (const xmlNodePtr xmlNode, Player * pPlayer)
    : RasterNode(xmlNode, pPlayer)
{
    m_href = getDefaultedStringAttr (xmlNode, "href", "");
    m_Hue = getDefaultedIntAttr (xmlNode, "hue", -1);
    m_Saturation = getDefaultedIntAttr (xmlNode, "saturation", -1);
    m_pBmp = BitmapPtr(new Bitmap(IntPoint(1,1), R8G8B8));
    load();
}

Image::~Image ()
{
}

void Image::connect(DisplayEngine * pEngine)
{
    RasterNode::connect(pEngine);

    setupSurface();
}

void Image::disconnect()
{
    // Unload textures but keep bitmap in memory.
    ISurface * pSurface = getSurface();
    m_pBmp = BitmapPtr(new Bitmap(*(pSurface->lockBmp())));
    // XXX Yuck
    if (!(getPlayer()->getDisplayEngine()->hasRGBOrdering())) {
        FilterFlipRGB().applyInPlace(m_pBmp);
    }
    getSurface()->unlockBmps();
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
    if (getState() == NS_CONNECTED) {
        setupSurface();
    }
    DPoint Size = getPreferredMediaSize();
    setViewport(-32767, -32767, Size.x, Size.y);
}

void Image::setBitmap(const Bitmap * pBmp)
{
    m_href = "";
    
    getSurface()->create(pBmp->getSize(), pBmp->getPixelFormat(), false);
    getSurface()->lockBmp()->copyPixels(*pBmp);
    getSurface()->unlockBmps();
    getEngine()->surfaceChanged(getSurface());
    
}

static ProfilingZone RenderProfilingZone("    Image::render");

void Image::render (const DRect& Rect)
{
    ScopeTimer Timer(RenderProfilingZone);
    getEngine()->blt32(getSurface(), &getAbsViewport(), getEffectiveOpacity(), 
            getAngle(), getPivot(), getBlendMode());
}

bool Image::obscures (const DRect& Rect, int Child) 
{
    return (isActive() && getEffectiveOpacity() > 0.999
            && !getSurface()->lockBmp()->hasAlpha() 
            && getVisibleRect().Contains(Rect));
}

string Image::getTypeStr ()
{
    return "Image";
}

DPoint Image::getPreferredMediaSize()
{
    if (getState() == NS_UNCONNECTED) {
        return DPoint(m_pBmp->getSize());
    } else {
        return DPoint(getSurface()->lockBmp()->getSize());
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
    }
    if (m_Saturation != -1) {
        FilterColorize(m_Hue, m_Saturation).applyInPlace(
                m_pBmp);
    }
}

void Image::setupSurface()
{
    PixelFormat pf;
    pf = R8G8B8;
    if (m_pBmp->hasAlpha()) {
        pf = R8G8B8A8;
    }
    getSurface()->create(m_pBmp->getSize(), pf, false);
    getSurface()->lockBmp()->copyPixels(*m_pBmp);
    if (!(getPlayer()->getDisplayEngine()->hasRGBOrdering())) {
        FilterFlipRGB().applyInPlace(getSurface()->lockBmp());
    }

    getSurface()->unlockBmps();
    getEngine()->surfaceChanged(getSurface());
    m_pBmp=BitmapPtr();
}

}
