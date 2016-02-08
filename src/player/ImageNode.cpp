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
#include "TypeRegistry.h"
#include "OGLSurface.h"
#include "Player.h"
#include "OffscreenCanvasNode.h"
#include "OffscreenCanvas.h"
#include "GPUImage.h"

#include "../base/Logger.h"
#include "../base/ScopeTimer.h"
#include "../base/XMLHelper.h"
#include "../base/Exception.h"
#include "../base/ObjectCounter.h"

#include "../graphics/Filterfliprgb.h"
#include "../graphics/MCTexture.h"
#include "../graphics/Bitmap.h"

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

ImageNode::ImageNode(const ArgList& args, const std::string& sPublisherName)
    : RasterNode(sPublisherName), m_Compression(TEXCOMPRESSION_NONE)
{
    args.setMembers(this);
    m_pGPUImage = GPUImagePtr(new GPUImage(getSurface(), getMipmap()));
    m_Compression = string2TexCompression(args.getArgVal<string>("compression"));
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
    if (m_pGPUImage->getSource() == GPUImage::SCENE) {
        checkCanvasValid(m_pGPUImage->getCanvas());
    }
    m_pGPUImage->moveToGPU();
    RasterNode::connectDisplay();
    if (m_pGPUImage->getSource() == GPUImage::SCENE) {
        m_pGPUImage->getCanvas()->addDependentCanvas(getCanvas());
    }
}

void ImageNode::connect(CanvasPtr pCanvas)
{
    RasterNode::connect(pCanvas);
    checkReload();
}

void ImageNode::disconnect(bool bKill)
{
    OffscreenCanvasPtr pCanvas = m_pGPUImage->getCanvas();
    if (pCanvas) {
        pCanvas->removeDependentCanvas(getCanvas());
    }
    if (bKill) {
        RasterNode::disconnect(bKill);
        m_pGPUImage = GPUImagePtr(new GPUImage(getSurface(), getMipmap()));
        m_href = "";
    } else {
        m_pGPUImage->moveToCPU();
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
    if (m_pGPUImage->getSource() == GPUImage::SCENE && getState() == Node::NS_CANRENDER)
    {
        m_pGPUImage->getCanvas()->removeDependentCanvas(getCanvas());
    }
    try {
        if (href == "") {
            m_pGPUImage->setEmpty();
        } else {
            checkReload();
        }
    } catch (const Exception&) {
        m_href = "";
        m_pGPUImage->setEmpty();
        throw;
    }
}

const string ImageNode::getCompression() const
{
    return texCompression2String(m_Compression);
}

void ImageNode::setBitmap(BitmapPtr pBmp)
{
    if (m_pGPUImage->getSource() == GPUImage::SCENE && getState() == Node::NS_CANRENDER) {
        m_pGPUImage->getCanvas()->removeDependentCanvas(getCanvas());
    }
    m_pGPUImage->setBitmap(pBmp, m_Compression);
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
    AreaNode::preRender(pVA, bIsParentActive, parentEffectiveOpacity);
    if (isVisible() && m_pGPUImage->getSource() != GPUImage::NONE) {
        if (m_pGPUImage->getCanvas()) {
            // Force FX render every frame for canvas nodes.
            getSurface()->setDirty();
        }
        scheduleFXRender();
    }
    calcVertexArray(pVA);
}

static ProfilingZoneID RenderProfilingZone("ImageNode::render");

void ImageNode::render(GLContext* pContext, const glm::mat4& transform)
{
    ScopeTimer Timer(RenderProfilingZone);
    if (m_pGPUImage->getSource() != GPUImage::NONE) {
        blt32(pContext, transform);
    }
}

IntPoint ImageNode::getMediaSize()
{
    return m_pGPUImage->getSize();
}

GPUImage::Source ImageNode::getSource() const
{
    return m_pGPUImage->getSource();
}

void ImageNode::checkReload()
{
    if (isCanvasURL(m_href)) {
        if (m_Compression != TEXCOMPRESSION_NONE) {
            throw Exception(AVG_ERR_UNSUPPORTED, 
                    "Texture compression can't be used with canvas hrefs.");
        }
        OffscreenCanvasPtr pCanvas = Player::get()->getCanvasFromURL(m_href);
        checkCanvasValid(pCanvas);
        m_pGPUImage->setCanvas(pCanvas);
        if (getState() == NS_CANRENDER) {
            pCanvas->addDependentCanvas(getCanvas());
        }
        newSurface();
    } else {
        bool bNewImage = Node::checkReload(m_href, m_pGPUImage, m_Compression);
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
        OffscreenCanvasPtr pCanvas = m_pGPUImage->getCanvas();
        if (pCanvas && pCanvas->getHandleEvents()) {
            glm::vec2 nodeSize(getSize());
            glm::vec2 canvasSize(pCanvas->getSize());
            glm::vec2 localPos(pos.x*(canvasSize.x/nodeSize.x), 
                    pos.y*(canvasSize.y/nodeSize.y));
            pCanvas->getRootNode()->getElementsByPos(localPos, pElements);
        }
        RasterNode::getElementsByPos(pos, pElements);
    }
}

glm::vec2 ImageNode::toCanvasPos(const glm::vec2& pos)
{
    OffscreenCanvasPtr pCanvas = m_pGPUImage->getCanvas();
    AVG_ASSERT(pCanvas);
    glm::vec2 containerPos = getRelPos(pos);
    glm::vec2 nodeSize(getSize());
    glm::vec2 canvasSize(pCanvas->getSize());
    return glm::vec2(containerPos.x*(canvasSize.x/nodeSize.x),
            containerPos.y*(canvasSize.y/nodeSize.y));
}

BitmapPtr ImageNode::getBitmap()
{
    return m_pGPUImage->getBitmap();
}

bool ImageNode::isCanvasURL(const std::string& sURL)
{
    return sURL.find("canvas:") == 0;
}

void ImageNode::checkCanvasValid(const CanvasPtr& pCanvas)
{
    if (pCanvas == getCanvas()) {
        m_href = "";
        m_pGPUImage->setEmpty();
        throw Exception(AVG_ERR_INVALID_ARGS, "Circular dependency between canvases.");
    }
}

}
