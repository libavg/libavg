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

#include "Canvas.h"

#include "Player.h"
#include "AVGNode.h"
#include "Shape.h"
#include "OffscreenCanvas.h"
#include "Window.h"

#include "../base/Exception.h"
#include "../base/Logger.h"
#include "../base/ScopeTimer.h"

#include "../graphics/StandardShader.h"
#include "../graphics/GLContextManager.h"
#include "../graphics/MCFBO.h"

#include <iostream>

using namespace std;
using namespace boost;

namespace avg {
        
Canvas::Canvas(Player * pPlayer)
    : m_pPlayer(pPlayer),
      m_bIsPlaying(false),
      m_PlaybackEndSignal(&IPlaybackEndListener::onPlaybackEnd),
      m_FrameEndSignal(&IFrameEndListener::onFrameEnd),
      m_PreRenderSignal(&IPreRenderListener::onPreRender),
      m_ClipLevel(0)
{
}

Canvas::~Canvas()
{
}

void Canvas::setRoot(NodePtr pRootNode)
{
    assert(!m_pRootNode);
    m_pRootNode = dynamic_pointer_cast<CanvasNode>(pRootNode);
    CanvasPtr pThis = dynamic_pointer_cast<Canvas>(shared_from_this());
    m_pRootNode->setParent(0, Node::NS_CONNECTED, pThis);
    registerNode(m_pRootNode);
}

void Canvas::initPlayback(int multiSampleSamples)
{
    m_bIsPlaying = true;
    m_pRootNode->connectDisplay();
    m_MultiSampleSamples = multiSampleSamples;
    m_pVertexArray = GLContextManager::get()->createVertexArray(2000, 3000);
}

void Canvas::stopPlayback(bool bIsAbort)
{
    if (m_bIsPlaying) {
        if (!bIsAbort) {
            m_PlaybackEndSignal.emit();
        }
        m_pRootNode->disconnect(true);
        m_pRootNode = CanvasNodePtr();
        m_IDMap.clear();
        m_bIsPlaying = false;
        m_pVertexArray = VertexArrayPtr();
    }
}

NodePtr Canvas::getElementByID(const std::string& id)
{
    if (m_IDMap.find(id) != m_IDMap.end()) {
        return m_IDMap.find(id)->second;
    } else {
        return NodePtr();
    }
}

void Canvas::registerNode(NodePtr pNode)
{
    addNodeID(pNode);    
    DivNodePtr pDivNode = boost::dynamic_pointer_cast<DivNode>(pNode);
    if (pDivNode) {
        for (unsigned i=0; i<pDivNode->getNumChildren(); i++) {
            registerNode(pDivNode->getChild(i));
        }
    }
}

void Canvas::addNodeID(NodePtr pNode)
{
    const string& id = pNode->getID();
    if (id != "") {
        if (m_IDMap.find(id) != m_IDMap.end() &&
            m_IDMap.find(id)->second != pNode)
        {
            throw (Exception (AVG_ERR_XML_DUPLICATE_ID,
                string("Error: duplicate id ")+id));
        }
        m_IDMap.insert(NodeIDMap::value_type(id, pNode));
    }
}

void Canvas::removeNodeID(const string& id)
{
    if (id != "") {
        map<string, NodePtr>::iterator it;
        it = m_IDMap.find(id);
        if (it != m_IDMap.end()) {
            m_IDMap.erase(it);
        } else {
            cerr << "removeNodeID(\"" << id << "\") failed." << endl;
            AVG_ASSERT(false);
        }
    }
}

CanvasNodePtr Canvas::getRootNode() const
{
    return m_pRootNode;
}

static ProfilingZoneID RenderProfilingZone("Render");

void Canvas::doFrame(bool bPythonAvailable)
{
    emitPreRenderSignal();
    if (!m_pPlayer->isStopping()) {
        ScopeTimer Timer(RenderProfilingZone);
        Player::get()->startTraversingTree();
        if (bPythonAvailable) {
            Py_BEGIN_ALLOW_THREADS;
            try {
                renderTree();
            } catch(...) {
                Py_BLOCK_THREADS;
                Player::get()->endTraversingTree();
                throw;
            }
            Py_END_ALLOW_THREADS;
        } else {
            renderTree();
        }
        Player::get()->endTraversingTree();
    }
    resetFXSchedule();
    emitFrameEndSignal();
}

IntPoint Canvas::getSize() const
{
    return IntPoint(m_pRootNode->getSize());
}
static ProfilingZoneID PushClipRectProfilingZone("pushClipRect");

void Canvas::pushClipRect(const glm::mat4& transform, SubVertexArray& va)
{
    ScopeTimer timer(PushClipRectProfilingZone);
    m_ClipLevel++;
    clip(transform, va, GL_INCR);
}

static ProfilingZoneID PopClipRectProfilingZone("popClipRect");

void Canvas::popClipRect(const glm::mat4& transform, SubVertexArray& va)
{
    ScopeTimer timer(PopClipRectProfilingZone);
    m_ClipLevel--;
    clip(transform, va, GL_DECR);
}

void Canvas::registerPlaybackEndListener(IPlaybackEndListener* pListener)
{
    m_PlaybackEndSignal.connect(pListener);
}

void Canvas::unregisterPlaybackEndListener(IPlaybackEndListener* pListener)
{
    m_PlaybackEndSignal.disconnect(pListener);
}

void Canvas::registerFrameEndListener(IFrameEndListener* pListener)
{
    m_FrameEndSignal.connect(pListener);
}

void Canvas::unregisterFrameEndListener(IFrameEndListener* pListener)
{
    m_FrameEndSignal.disconnect(pListener);
}

void Canvas::registerPreRenderListener(IPreRenderListener* pListener)
{
    m_PreRenderSignal.connect(pListener);
}

void Canvas::unregisterPreRenderListener(IPreRenderListener* pListener)
{
    m_PreRenderSignal.disconnect(pListener);
}

Player* Canvas::getPlayer() const
{
    return m_pPlayer;
}

vector<NodePtr> Canvas::getElementsByPos(const glm::vec2& pos) const
{
    vector<NodePtr> elements;
    m_pRootNode->getElementsByPos(pos, elements);
    return elements;
}

static ProfilingZoneID PreRenderProfilingZone("PreRender");
static ProfilingZoneID VATransferProfilingZone("VA Transfer");

void Canvas::preRender()
{
    ScopeTimer Timer(PreRenderProfilingZone);
    m_pVertexArray->reset();
    m_pRootNode->preRender(m_pVertexArray, true, 1.0f);
}

static ProfilingZoneID RootRenderProfilingZone("RootNode: render");

void Canvas::renderWindow(WindowPtr pWindow, MCFBOPtr pFBO, const IntRect& viewport)
{
    pWindow->getGLContext()->activate();
    GLContextManager::get()->uploadDataForContext();
    renderFX();
    glm::mat4 projMat;
    if (pFBO) {
        pFBO->activate();
        glm::vec2 size = m_pRootNode->getSize();
        projMat = glm::ortho(0.f, size.x, 0.f, size.y);
        glViewport(0, 0, GLsizei(size.x), GLsizei(size.y));
    } else {
        glproc::BindFramebuffer(GL_FRAMEBUFFER, 0);
        projMat = glm::ortho(float(viewport.tl.x), float(viewport.br.x), 
                float(viewport.br.y), float(viewport.tl.y));
        IntPoint windowSize = pWindow->getSize();
        glViewport(0, 0, windowSize.x, windowSize.y);
    }
    {
        ScopeTimer Timer(VATransferProfilingZone);
        m_pVertexArray->update();
    }
    clearGLBuffers(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT,
            !pFBO);
    GLContext::checkError("Canvas::renderWindow: glViewport()");
    m_pVertexArray->activate();
    {
        ScopeTimer timer(RootRenderProfilingZone);
        m_pRootNode->maybeRender(projMat);
    }
    renderOutlines(projMat);
}

void Canvas::scheduleFXRender(const RasterNodePtr& pNode)
{
    m_pScheduledFXNodes.push_back(pNode);
}

void Canvas::renderOutlines(const glm::mat4& transform)
{
    GLContext* pContext = GLContext::getCurrent();
    VertexArrayPtr pVA = GLContextManager::get()->createVertexArray();
    pVA->initForGLContext();
    pContext->setBlendMode(GLContext::BLEND_BLEND, false);
    m_pRootNode->renderOutlines(pVA, Pixel32(0,0,0,0));
    StandardShaderPtr pShader = pContext->getStandardShader();
    pShader->setTransform(transform);
    pShader->setUntextured();
    pShader->setAlpha(0.5f);
    pShader->activate();
    if (pVA->getNumVerts() != 0) {
        pVA->draw();
    }
}

void Canvas::renderFX()
{
    vector<RasterNodePtr>::iterator it;
    for (it=m_pScheduledFXNodes.begin(); it!=m_pScheduledFXNodes.end(); ++it) {
        (*it)->renderFX();
    }
}

void Canvas::resetFXSchedule()
{
    vector<RasterNodePtr>::iterator it;
    for (it=m_pScheduledFXNodes.begin(); it!=m_pScheduledFXNodes.end(); ++it) {
        (*it)->resetFXDirty();
    }
    m_pScheduledFXNodes.clear();
}


void Canvas::clip(const glm::mat4& transform, SubVertexArray& va, GLenum stencilOp)
{
    // Disable drawing to color buffer
    glColorMask(0, 0, 0, 0);

    // Enable drawing to stencil buffer
    glStencilMask(~0);

    // Draw clip rectangle into stencil buffer
    glStencilFunc(GL_ALWAYS, 0, 0);
    glStencilOp(stencilOp, stencilOp, stencilOp);

    StandardShaderPtr pShader = GLContext::getCurrent()->getStandardShader();
    pShader->setUntextured();
    pShader->setTransform(transform);
    pShader->activate();
    va.draw();

    // Set stencil test
    glStencilFunc(GL_LEQUAL, m_ClipLevel, ~0);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

    // Disable drawing to stencil buffer
    glStencilMask(0);

    // Enable drawing to color buffer
    glColorMask(~0, ~0, ~0, ~0);
}

static ProfilingZoneID PreRenderSignalProfilingZone("PreRender signal");

void Canvas::emitPreRenderSignal()
{
    ScopeTimer Timer(PreRenderSignalProfilingZone);
    m_PreRenderSignal.emit();
}

static ProfilingZoneID FrameEndProfilingZone("OnFrameEnd");

void Canvas::emitFrameEndSignal()
{
    ScopeTimer Timer(FrameEndProfilingZone);
    m_FrameEndSignal.emit();
}

}
