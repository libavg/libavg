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

#include "ImageNode.h"

#include "SDLDisplayEngine.h"
#include "NodeDefinition.h"
#include "OGLSurface.h"
#include "Player.h"
#include "OffscreenCanvas.h"

#include "../base/Logger.h"
#include "../base/ScopeTimer.h"
#include "../base/XMLHelper.h"
#include "../base/Exception.h"
#include "../base/ObjectCounter.h"

#include "../graphics/Filterfliprgb.h"

#include <iostream>
#include <sstream>

using namespace std;

namespace avg {

NodeDefinition ImageNode::createDefinition()
{
    return NodeDefinition("image", VisibleNode::buildNode<ImageNode>)
        .extendDefinition(RasterNode::createDefinition())
        .addArg(Arg<UTF8String>("href", "", false, offsetof(ImageNode, m_href)))
        .addArg(Arg<string>("compression", "none"));
}

ImageNode::ImageNode(const ArgList& args)
    : m_Compression(Image::TEXTURECOMPRESSION_NONE)
{
    m_pImage = ImagePtr(new Image(getSurface()));
    args.setMembers(this);
    m_Compression = Image::string2compression(args.getArgVal<string>("compression"));
    setHRef(m_href);
    ObjectCounter::get()->incRef(&typeid(*this));
}

ImageNode::~ImageNode()
{
    // XXX: The following assert checks that disconnect(true) has been called.
    // However, if connect() has been called but rendering never started, 
    // disconnect isn't called either.
//    AVG_ASSERT(!m_pImage->getCanvas());
    ObjectCounter::get()->decRef(&typeid(*this));
}

void ImageNode::setRenderingEngines(DisplayEngine * pDisplayEngine,
        AudioEngine * pAudioEngine)
{
    getSurface()->attach(dynamic_cast<SDLDisplayEngine*>(pDisplayEngine));
    m_pImage->moveToGPU(dynamic_cast<SDLDisplayEngine*>(pDisplayEngine));
    RasterNode::setRenderingEngines(pDisplayEngine, pAudioEngine);
    if (m_pImage->getSource() == Image::SCENE) {
        m_pImage->getCanvas()->addDependentCanvas(getCanvas());
    }
}

void ImageNode::connect(CanvasPtr pCanvas)
{
    RasterNode::connect(pCanvas);
    checkReload();
}

void ImageNode::disconnect(bool bKill)
{
    OffscreenCanvasPtr pCanvas = m_pImage->getCanvas();
    if (pCanvas) {
        pCanvas->removeDependentCanvas(getCanvas());
    }
    if (bKill) {
        RasterNode::disconnect(bKill);
        m_pImage = ImagePtr(new Image(getSurface()));
        m_href = "";
    } else {
        m_pImage->moveToCPU();
        RasterNode::disconnect(bKill);
    }
}

const UTF8String& ImageNode::getHRef() const
{
    return m_href;
}

void ImageNode::setHRef(const UTF8String& href)
{
    m_href = href;
    if (m_pImage->getSource() == Image::SCENE && getState() == VisibleNode::NS_CANRENDER)
    {
        m_pImage->getCanvas()->removeDependentCanvas(getCanvas());
    }
    try {
        if (href == "") {
            m_pImage->setEmpty();
        } else {
            checkReload();
        }
    } catch (const Exception&) {
        m_href = "";
        m_pImage->setEmpty();
        throw;
    }
}

const string ImageNode::getCompression() const
{
    return Image::compression2String(m_Compression);
}

void ImageNode::setBitmap(const Bitmap * pBmp)
{
    if (m_pImage->getSource() == Image::SCENE && getState() == VisibleNode::NS_CANRENDER)
    {
        m_pImage->getCanvas()->removeDependentCanvas(getCanvas());
    }
    m_pImage->setBitmap(pBmp, m_Compression);
    if (getState() == VisibleNode::NS_CANRENDER) {
        bind();
    }
    m_href = "";
    setViewport(-32767, -32767, -32767, -32767);
}

static ProfilingZoneID RenderProfilingZone("ImageNode::render");

void ImageNode::preRender()
{
    VisibleNode::preRender();
    renderFX(getSize(), Pixel32(255, 255, 255, 255), false);
}

void ImageNode::render(const DRect& Rect)
{
    ScopeTimer Timer(RenderProfilingZone);
    if (m_pImage->getSource() != Image::NONE) {
        blt32(getSize(), getEffectiveOpacity(), getBlendMode(), 
                bool(m_pImage->getCanvas()));
    }
}

IntPoint ImageNode::getMediaSize()
{
    return m_pImage->getSize();
}

void ImageNode::checkReload()
{
    if (isCanvasURL(m_href)) {
        if (m_Compression != Image::TEXTURECOMPRESSION_NONE) {
            throw Exception(AVG_ERR_UNSUPPORTED, 
                    "Texture compression can't be used with canvas hrefs.");
        }
        OffscreenCanvasPtr pCanvas = Player::get()->getCanvasFromURL(m_href);
        m_pImage->setCanvas(pCanvas);
        if (getState() == NS_CANRENDER) {
            pCanvas->addDependentCanvas(getCanvas());
        }
    } else {
        VisibleNode::checkReload(m_href, m_pImage, m_Compression);
    }
    setViewport(-32767, -32767, -32767, -32767);
    RasterNode::checkReload();
}

void ImageNode::getElementsByPos(const DPoint& pos, 
                vector<VisibleNodeWeakPtr>& pElements)
{
    if (reactsToMouseEvents()) {
        OffscreenCanvasPtr pCanvas = m_pImage->getCanvas();
        if (pCanvas && pCanvas->getHandleEvents()) {
            DPoint nodeSize(getSize());
            DPoint canvasSize(pCanvas->getSize());
            DPoint localPos(pos.x*(canvasSize.x/nodeSize.x), 
                    pos.y*(canvasSize.y/nodeSize.y));
            pCanvas->getRootNode()->getElementsByPos(localPos, pElements);
        } else {
            RasterNode::getElementsByPos(pos, pElements);
        }
    }
}

BitmapPtr ImageNode::getBitmap()
{
    return m_pImage->getBitmap();
}

bool ImageNode::isCanvasURL(const std::string& sURL)
{
    return sURL.find("canvas:") == 0;
}

}
