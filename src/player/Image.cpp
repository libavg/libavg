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
#include "NodeDefinition.h"

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

NodeDefinition Image::getNodeDefinition()
{
    return NodeDefinition("image", Node::buildNode<Image>)
        .extendDefinition(RasterNode::getNodeDefinition())
        .addArg(Arg<string>("href", "", false, offsetof(Image, m_href)))
        .addArg(Arg<int>("hue", -1, false, offsetof(Image, m_Hue)))
        .addArg(Arg<int>("saturation", -1, false, offsetof(Image, m_Saturation)));
}

Image::Image (const ArgList& Args, Player * pPlayer)
    : RasterNode(pPlayer),
      m_bIsImageAvailable(false)
{
    Args.setMembers(this);
    setHRef(m_href);
}

Image::~Image ()
{
}

void Image::setRenderingEngines(DisplayEngine * pDisplayEngine, AudioEngine * pAudioEngine)
{
    checkReload();
    RasterNode::setRenderingEngines(pDisplayEngine, pAudioEngine);
    setupSurface(&*m_pBmp);
}

void Image::disconnect()
{
    // Commenting this (and the corresponding line in setupSurface) out causes 
    // a copy of the image to always be kept in main memory, so no readback has to
    // take place.

    if (isDisplayAvailable()) {
        // Unload textures but keep bitmap in memory.
        ISurface * pSurface = getSurface();
        BitmapPtr pSurfaceBmp = pSurface->lockBmp();
        m_pBmp = BitmapPtr(new Bitmap(pSurfaceBmp->getSize(), pSurfaceBmp->getPixelFormat()));
        m_pBmp->copyPixels(*pSurfaceBmp);
        getSurface()->unlockBmps();
#ifdef __i386__
        // XXX Yuck
        if (!(getPlayer()->getDisplayEngine()->hasRGBOrdering()) && 
            m_pBmp->getBytesPerPixel() >= 3)
        {
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
    }
    DPoint Size = getPreferredMediaSize();
    setViewport(-32767, -32767, Size.x, Size.y);
}

void Image::setBitmap(const Bitmap * pBmp)
{
    // TODO: Add a unique bitmap identifier to the URI.
    m_bIsImageAvailable = true;
    m_href = "";
    m_Filename = "";
    PixelFormat pf;
    pf = R8G8B8X8;
    if (pBmp->hasAlpha()) {
        pf = R8G8B8A8;
    }
    if (pBmp->getPixelFormat() == I8) {
        pf = I8;
    }
#if defined(__i386__) || defined(_WIN32)
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
#endif
//    cerr << "setBitmap, pf: " << Bitmap::getPixelFormatString(pf) << endl;
    if (isDisplayAvailable()) {
        ISurface * pSurface = getSurface();
        if (pSurface->getSize() != pBmp->getSize() || pSurface->getPixelFormat() != pf) {
            pSurface->create(pBmp->getSize(), pf, true);
        }
        BitmapPtr pSurfaceBmp = getSurface()->lockBmp();
        pSurfaceBmp->copyPixels(*pBmp);
        getSurface()->unlockBmps();
        getDisplayEngine()->surfaceChanged(getSurface());
    } else {
        if (m_pBmp->getSize() != pBmp->getSize() || m_pBmp->getPixelFormat() != pf) {
            m_pBmp = BitmapPtr(new Bitmap(pBmp->getSize(), pf, ""));
        }
        m_pBmp->copyPixels(*pBmp);
    }
    DPoint Size = getPreferredMediaSize();
    setViewport(-32767, -32767, Size.x, Size.y);
}

static ProfilingZone RenderProfilingZone("Image::render");

void Image::render (const DRect& Rect)
{
    ScopeTimer Timer(RenderProfilingZone);
    if (m_bIsImageAvailable) {
        getDisplayEngine()->blt32(getSurface(), getRelSize(), 
                getEffectiveOpacity(), getBlendMode());
    }
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

void Image::checkReload()
{
    string sLastFilename = m_Filename;
    m_Filename = m_href;
    if (m_Filename != "") {
        initFilename(getPlayer(), m_Filename);
    }
    if (sLastFilename != m_Filename || !m_pBmp) {
        load();
        if (isDisplayAvailable()) {
            setupSurface(&*m_pBmp);
        }
        DPoint Size = getPreferredMediaSize();
        setViewport(-32767, -32767, Size.x, Size.y);
    }
}

Bitmap * Image::getBitmap()
{
    if (isDisplayAvailable()) {
        return RasterNode::getBitmap();
    } else {
        Bitmap * pBmp;
        pBmp = new Bitmap(*m_pBmp);
        return pBmp;
    }
}

void Image::load()
{
    m_Filename = m_href;
    m_pBmp = BitmapPtr(new Bitmap(IntPoint(1,1), R8G8B8X8));
    m_bIsImageAvailable = false;
    if (m_Filename != "") {
        initFilename(getPlayer(), m_Filename);
        AVG_TRACE(Logger::MEMORY, "Loading " << m_Filename);
        try {
            m_pBmp = BitmapPtr(new Bitmap(m_Filename));
            m_bIsImageAvailable = true;
        } catch (Magick::Exception & ex) {
            if (getState() == Node::NS_CONNECTED) {
                AVG_TRACE(Logger::ERROR, ex.what());
            }
        }
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
#if defined __APPLE__ || defined _WIN32
    if (!getSurface()->isOneTexture(pBmp->getSize())) {
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
    getDisplayEngine()->surfaceChanged(getSurface());
    m_pBmp=BitmapPtr();
}

}
