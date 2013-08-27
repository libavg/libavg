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

#include "OffscreenCanvas.h"

#include "CanvasNode.h"
#include "Player.h"

#include "../base/Exception.h"
#include "../base/ProfilingZoneID.h"
#include "../base/ObjectCounter.h"
#include "../base/ScopeTimer.h"

#include "../graphics/FilterUnmultiplyAlpha.h"
#include "../graphics/BitmapLoader.h"

#include <iostream>

using namespace boost;
using namespace std;

namespace avg {
    
OffscreenCanvas::OffscreenCanvas(Player * pPlayer)
    : Canvas(pPlayer),
      m_bIsRendered(false),
      m_pCameraNodeRef(0)
{
    ObjectCounter::get()->incRef(&typeid(*this));
}

OffscreenCanvas::~OffscreenCanvas()
{
    ObjectCounter::get()->decRef(&typeid(*this));
}

void OffscreenCanvas::setRoot(NodePtr pRootNode)
{
    Canvas::setRoot(pRootNode);
    if (!getRootNode()) {
        throw (Exception(AVG_ERR_XML_PARSE,
                    "Root node of a canvas tree needs to be a <canvas> node."));
    }
}

void OffscreenCanvas::initPlayback()
{
    m_bUseMipmaps = getMipmap();
    PixelFormat pf;
    if (BitmapLoader::get()->isBlueFirst()) {
        pf = B8G8R8A8;
    } else {
        pf = R8G8B8A8;
    }
    bool bUseDepthBuffer = GLContext::getMain()->useDepthBuffer();
    m_pFBO = FBOPtr(new FBO(getSize(), pf, 1, getMultiSampleSamples(), bUseDepthBuffer,
            true, m_bUseMipmaps));
    Canvas::initPlayback(getMultiSampleSamples());
    m_bIsRendered = false;
}

void OffscreenCanvas::stopPlayback(bool bIsAbort)
{
    m_pFBO = FBOPtr();
    Canvas::stopPlayback(bIsAbort);
    m_bIsRendered = false;
}

BitmapPtr OffscreenCanvas::screenshot() const
{
    BitmapPtr pBmp = screenshotIgnoreAlpha();
    FilterUnmultiplyAlpha().applyInPlace(pBmp);
    return pBmp;
}

BitmapPtr OffscreenCanvas::screenshotIgnoreAlpha() const
{
    if (!isRunning() || !m_bIsRendered) {
        throw(Exception(AVG_ERR_UNSUPPORTED,
                "OffscreenCanvas::screenshot(): Canvas has not been rendered. No screenshot available"));
    }
    BitmapPtr pBmp = m_pFBO->getImage(0);
    return pBmp;
}

bool OffscreenCanvas::getHandleEvents() const
{
    return dynamic_pointer_cast<OffscreenCanvasNode>(getRootNode())->getHandleEvents();
}

int OffscreenCanvas::getMultiSampleSamples() const
{
    return dynamic_pointer_cast<OffscreenCanvasNode>(
            getRootNode())->getMultiSampleSamples();
}

bool OffscreenCanvas::getMipmap() const
{
    return dynamic_pointer_cast<OffscreenCanvasNode>(getRootNode())->getMipmap();
}

bool OffscreenCanvas::getAutoRender() const
{
    return dynamic_pointer_cast<OffscreenCanvasNode>(getRootNode())->getAutoRender();
}

void OffscreenCanvas::setAutoRender(bool bAutoRender)
{
    dynamic_pointer_cast<OffscreenCanvasNode>(getRootNode())->setAutoRender(bAutoRender);
}

void OffscreenCanvas::manualRender()
{
    emitPreRenderSignal(); 
    renderTree(); 
    emitFrameEndSignal(); 
}

std::string OffscreenCanvas::getID() const
{
    return getRootNode()->getID();
}

bool OffscreenCanvas::isRunning() const
{
    return (m_pFBO != FBOPtr());
}

GLTexturePtr OffscreenCanvas::getTex() const
{
    AVG_ASSERT(isRunning());
    return m_pFBO->getTex();
}

FBOPtr OffscreenCanvas::getFBO() const
{
    AVG_ASSERT(isRunning());
    return m_pFBO;
}

void OffscreenCanvas::registerCameraNode(CameraNode* pCameraNode)
{
    m_pCameraNodeRef = pCameraNode;
    m_pCameraNodeRef->setAutoUpdateCameraImage(false);
}

void OffscreenCanvas::unregisterCameraNode()
{
    m_pCameraNodeRef->setAutoUpdateCameraImage(true);
    m_pCameraNodeRef = NULL;
}

void OffscreenCanvas::updateCameraImage()
{
    m_pCameraNodeRef->updateCameraImage();
}

bool OffscreenCanvas::hasRegisteredCamera() const
{
    return m_pCameraNodeRef != NULL;
}

bool OffscreenCanvas::isCameraImageAvailable() const
{
    if (!hasRegisteredCamera()) {
        return false;
    }
    return m_pCameraNodeRef->isImageAvailable();
}

void OffscreenCanvas::addDependentCanvas(CanvasPtr pCanvas)
{
    m_pDependentCanvases.push_back(pCanvas);
    try {
        Player::get()->newCanvasDependency();
    } catch (Exception&) {
        m_pDependentCanvases.pop_back();
        throw;
    }
}

void OffscreenCanvas::removeDependentCanvas(CanvasPtr pCanvas)
{
    for (unsigned i = 0; i < m_pDependentCanvases.size(); ++i) {
        if (pCanvas == m_pDependentCanvases[i]) {
            m_pDependentCanvases.erase(m_pDependentCanvases.begin()+i);
//            dump();
            return;
        }
    }
    AVG_ASSERT(false);
}

const vector<CanvasPtr>& OffscreenCanvas::getDependentCanvases() const
{
    return m_pDependentCanvases;
}

unsigned OffscreenCanvas::getNumDependentCanvases() const
{
    return m_pDependentCanvases.size();
}

bool OffscreenCanvas::isSupported()
{
    if (!Player::get()->isPlaying()) {
        throw(Exception(AVG_ERR_UNSUPPORTED, 
                "OffscreenCanvas::isSupported(): Player.play() needs to be called before support can be queried."));
    }
    if (GLContext::getMain()->isGLES()) {
        return true;
    } else {
        return FBO::isFBOSupported() && FBO::isPackedDepthStencilSupported();
    }
}

bool OffscreenCanvas::isMultisampleSupported()
{
    if (!Player::get()->isPlaying()) {
        throw(Exception(AVG_ERR_UNSUPPORTED, 
                "OffscreenCanvas::isMultisampleSupported(): Player.play() needs to be called before multisample support can be queried."));
    }

    return FBO::isMultisampleFBOSupported();
}

void OffscreenCanvas::dump() const
{
    cerr << "Canvas: " << getRootNode()->getID() << endl;
    for (unsigned i = 0; i < m_pDependentCanvases.size(); ++i) {
        cerr << " " << m_pDependentCanvases[i]->getRootNode()->getID() << endl;
    }
}

static ProfilingZoneID OffscreenRenderProfilingZone("Render OffscreenCanvas");

void OffscreenCanvas::renderTree()
{
    if (!isRunning()) {
        throw(Exception(AVG_ERR_UNSUPPORTED, 
                "OffscreenCanvas::renderTree(): Player.play() needs to be called before rendering offscreen canvases."));
    }
    preRender();
    m_pFBO->activate();
    {
        ScopeTimer Timer(OffscreenRenderProfilingZone);
        Canvas::render(IntPoint(getRootNode()->getSize()), true);
    }
    m_pFBO->copyToDestTexture();
    m_bIsRendered = true;
}

}
