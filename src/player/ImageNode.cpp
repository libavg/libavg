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

#include "ImageNode.h"

#include "TypeDefinition.h"
#include "OGLSurface.h"
#include "Player.h"
#include "OffscreenCanvas.h"

#include "../base/Logger.h"
#include "../base/ScopeTimer.h"
#include "../base/XMLHelper.h"
#include "../base/Exception.h"
#include "../base/ObjectCounter.h"

#include "../graphics/Filterfliprgb.h"
#include "../graphics/MCTexture.h"

#include <iostream>
#include <sstream>

using namespace std;
using namespace boost;

namespace avg {

void ImageNode::registerType()
{
    TypeDefinition def = TypeDefinition("image", "rasternode", 
            ExportedObject::buildObject<ImageNode>)
        .addArg(Arg<UTF8String>("href", "", false, offsetof(ImageNode, m_href)))
        .addArg(Arg<string>("compression", "none"));
    TypeRegistry::get()->registerType(def);
}

ImageNode::ImageNode(const ArgList& args)
    : m_Compression(Image::TEXTURECOMPRESSION_NONE)
{
    args.setMembers(this);
    m_pImage = ImagePtr(new Image(getSurface(), getMaterial()));
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

void ImageNode::connectDisplay()
{
    if (m_pImage->getSource() == Image::SCENE) {
        checkCanvasValid(m_pImage->getCanvas());
    }
    m_pImage->moveToGPU();
    RasterNode::connectDisplay();
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
        m_pImage = ImagePtr(new Image(getSurface(), getMaterial()));
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
    if (m_pImage->getSource() == Image::SCENE && getState() == Node::NS_CANRENDER)
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

void ImageNode::setBitmap(BitmapPtr pBmp)
{
    if (m_pImage->getSource() == Image::SCENE && getState() == Node::NS_CANRENDER) {
        m_pImage->getCanvas()->removeDependentCanvas(getCanvas());
    }
    m_pImage->setBitmap(pBmp, m_Compression);
    if (getState() == Node::NS_CANRENDER) {
        newSurface();
    }
    m_href = "";
    setViewport(-32767, -32767, -32767, -32767);
}

static ProfilingZoneID PrerenderProfilingZone("ImageNode::prerender");

void ImageNode::preRender(const VertexArrayPtr& pVA, bool bIsParentActive, 
        float parentEffectiveOpacity)
{
    ScopeTimer timer(PrerenderProfilingZone);
    Node::preRender(pVA, bIsParentActive, parentEffectiveOpacity);
    if (isVisible() && m_pImage->getSource() != Image::NONE) {
        if (m_pImage->getCanvas()) {
            // Force FX render every frame for canvas nodes.
            getSurface()->getTex(0)->setDirty();
        }
        scheduleFXRender();
    }
    calcVertexArray(pVA);
}

static ProfilingZoneID RenderProfilingZone("ImageNode::render");

void ImageNode::render()
{
    ScopeTimer Timer(RenderProfilingZone);
    if (m_pImage->getSource() != Image::NONE) {
        blt32();
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
        checkCanvasValid(pCanvas);
        m_pImage->setCanvas(pCanvas);
        if (getState() == NS_CANRENDER) {
            pCanvas->addDependentCanvas(getCanvas());
        }
        newSurface();
    } else {
        bool bNewImage = Node::checkReload(m_href, m_pImage, m_Compression);
        if (bNewImage) {
            newSurface();
        }
    }
    setViewport(-32767, -32767, -32767, -32767);
    RasterNode::checkReload();
}

void ImageNode::getElementsByPos(const glm::vec2& pos, vector<NodePtr>& pElements)
{
    if (reactsToMouseEvents()) {
        OffscreenCanvasPtr pCanvas = m_pImage->getCanvas();
        if (pCanvas && pCanvas->getHandleEvents()) {
            glm::vec2 nodeSize(getSize());
            glm::vec2 canvasSize(pCanvas->getSize());
            glm::vec2 localPos(pos.x*(canvasSize.x/nodeSize.x), 
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

void ImageNode::checkCanvasValid(const CanvasPtr& pCanvas)
{
    if (pCanvas == getCanvas()) {
        m_href = "";
        m_pImage->setEmpty();
        throw Exception(AVG_ERR_INVALID_ARGS,
                "Circular dependency between canvases.");
    }
}

}
