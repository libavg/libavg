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

#include "OffscreenCanvas.h"

#include "SDLDisplayEngine.h"
#include "CanvasNode.h"
#include "Player.h"

#include "../base/Exception.h"
#include "../base/ProfilingZone.h"
#include "../base/ObjectCounter.h"

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

void OffscreenCanvas::initPlayback(SDLDisplayEngine* pDisplayEngine, 
        AudioEngine* pAudioEngine)
{
    Canvas::initPlayback(pDisplayEngine, pAudioEngine, getMultiSampleSamples());
    m_bUseMipmaps = getMipmap();
    m_pFBO = FBOPtr(new FBO(getSize(), B8G8R8X8, 1, getMultiSampleSamples(), true,
            m_bUseMipmaps));
    glEnable(GL_STENCIL_TEST);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    m_bIsRendered = false;
}

void OffscreenCanvas::stopPlayback()
{
    m_pFBO = FBOPtr();
    Canvas::stopPlayback();
    m_bIsRendered = false;
}

static ProfilingZone OffscreenRenderProfilingZone("Render OffscreenCanvas");

void OffscreenCanvas::render()
{
    if (!isRunning()) {
        throw(Exception(AVG_ERR_UNSUPPORTED, 
                "OffscreenCanvas::screenshot(): Player.play() needs to be called before rendering offscreen canvases."));
    }
    getDisplayEngine()->setMainFBO(m_pFBO);
    m_pFBO->activate();
    Canvas::render(IntPoint(getRootNode()->getSize()), true, 
            OffscreenRenderProfilingZone);
    m_pFBO->deactivate();
    m_pFBO->copyToDestTexture();
    getDisplayEngine()->setMainFBO(FBOPtr());
    m_bIsRendered = true;
}

BitmapPtr OffscreenCanvas::screenshot() const
{
    if (!isRunning() || !m_bIsRendered) {
        throw(Exception(AVG_ERR_UNSUPPORTED,
                "OffscreenCanvas::screenshot(): Canvas has not been rendered. No screenshot available"));
    }
    return m_pFBO->getImage(0);
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
    Player::get()->newCanvasDependency(
            dynamic_pointer_cast<OffscreenCanvas>(shared_from_this()));
}

void OffscreenCanvas::removeDependentCanvas(CanvasPtr pCanvas)
{
    for (unsigned i=0; i<m_pDependentCanvases.size(); ++i) {
        if (pCanvas == m_pDependentCanvases[i]) {
            m_pDependentCanvases.erase(m_pDependentCanvases.begin()+i);
//            dump();
            return;
        }
    }
    AVG_ASSERT(false);
}

bool OffscreenCanvas::hasDependentCanvas(CanvasPtr pCanvas) const
{
    for (unsigned i=0; i<m_pDependentCanvases.size(); ++i) {
        if (pCanvas == m_pDependentCanvases[i]) {
            return true;
        }
    }
    return false;
}

unsigned OffscreenCanvas::getNumDependentCanvases() const
{
    return m_pDependentCanvases.size();
}

bool OffscreenCanvas::isMultisampleSupported()
{
    return FBO::isMultisampleFBOSupported();
}

void OffscreenCanvas::dump() const
{
    cerr << "Canvas: " << getRootNode()->getID() << endl;
    for (unsigned i=0; i<m_pDependentCanvases.size(); ++i) {
        cerr << " " << m_pDependentCanvases[i]->getRootNode()->getID() << endl;
    }
}

}
