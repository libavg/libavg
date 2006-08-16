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

#include "../graphics/Bitmap.h"
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
}

Image::Image (const xmlNodePtr xmlNode, DivNode * pParent)
    : RasterNode(xmlNode, pParent)
{
    m_href = getDefaultedStringAttr (xmlNode, "href", "");
    m_Hue = getDefaultedIntAttr (xmlNode, "hue", -1);
    m_Saturation = getDefaultedIntAttr (xmlNode, "saturation", -1);
    
}

Image::~Image ()
{
}

void Image::init (DisplayEngine * pEngine, DivNode * pParent,
        Player * pPlayer)
{
    Node::init(pEngine, pParent, pPlayer);
    m_pPlayer = pPlayer;
    load();
}

const std::string& Image::getHRef() const
{
    return m_href;
}

void Image::setHRef(const string& href)
{
    m_href = href;
    load();
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
    return DPoint(getSurface()->lockBmp()->getSize());
}

void Image::load()
{
    m_Filename = m_href;
    if (m_Filename != "") {
        initFilename(m_pPlayer, m_Filename);
        //    AVG_TRACE(Logger::PROFILE, "Loading " << m_Filename);
        try {
            Bitmap TempBmp(m_Filename);
            PixelFormat pf;
            pf = R8G8B8;
            if (TempBmp.hasAlpha()) {
                pf = R8G8B8A8;
            }
            getSurface()->create(TempBmp.getSize(), pf, false);
            getSurface()->lockBmp()->copyPixels(TempBmp);
        } catch (Magick::Exception & ex) {
            AVG_TRACE(Logger::ERROR, ex.what());
            getSurface()->create(IntPoint(1,1), R8G8B8, false);
        }
    } else {
        getSurface()->create(IntPoint(1,1), R8G8B8, false);
    }
    
    if (m_Saturation != -1) {
        FilterColorize(m_Hue, m_Saturation).applyInPlace(
                getSurface()->lockBmp());
    }
    if (!(m_pPlayer->getDisplayEngine()->hasRGBOrdering())) {
        FilterFlipRGB().applyInPlace(getSurface()->lockBmp());
    }
    getSurface()->unlockBmps();
    getEngine()->surfaceChanged(getSurface());
}

}
