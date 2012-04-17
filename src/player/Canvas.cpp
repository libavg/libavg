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

#include "Canvas.h"

#include "Player.h"
#include "AVGNode.h"
#include "Shape.h"
#include "OffscreenCanvas.h"

#include "../base/Exception.h"
#include "../base/Logger.h"
#include "../base/ScopeTimer.h"

#include "../graphics/StandardShader.h"

#include <iostream>

using namespace std;
using namespace boost;

namespace avg {
        
CanvasPtr Canvas::s_pActiveCanvas;

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
    m_pRootNode->setParent(DivNodeWeakPtr(), Node::NS_CONNECTED,
            shared_from_this());
    registerNode(m_pRootNode);
}

void Canvas::initPlayback(int multiSampleSamples)
{
    m_bIsPlaying = true;
    m_pRootNode->connectDisplay();
    m_MultiSampleSamples = multiSampleSamples;
}

void Canvas::stopPlayback()
{
    if (m_bIsPlaying) {
        m_PlaybackEndSignal.emit();
        m_pRootNode->disconnect(true);
        m_pRootNode = CanvasNodePtr();
        m_IDMap.clear();
        m_bIsPlaying = false;
    }
}

NodePtr Canvas::getElementByID(const std::string& id)
{
    if (m_IDMap.find(id) != m_IDMap.end()) {
        return m_IDMap.find(id)->second;
    } else {
        AVG_TRACE(Logger::WARNING, "getElementByID(\"" << id << "\") failed.");
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
    s_pActiveCanvas = shared_from_this();
    emitPreRenderSignal();
    if (!m_pPlayer->isStopping()) {
        ScopeTimer Timer(RenderProfilingZone);
        if (bPythonAvailable) {
            Py_BEGIN_ALLOW_THREADS;
            try {
                render();
            } catch(...) {
                Py_BLOCK_THREADS;
                throw;
            }
            Py_END_ALLOW_THREADS;
        } else {
            render();
        }
    }
    emitFrameEndSignal();
    s_pActiveCanvas = CanvasPtr();
}

IntPoint Canvas::getSize() const
{
    return IntPoint(m_pRootNode->getSize());
}
static ProfilingZoneID PushClipRectProfilingZone("pushClipRect");

void Canvas::pushClipRect(const glm::mat4& transform, VertexArrayPtr pVA)
{
    ScopeTimer timer(PushClipRectProfilingZone);
    m_ClipLevel++;
    clip(transform, pVA, GL_INCR);
}

static ProfilingZoneID PopClipRectProfilingZone("popClipRect");

void Canvas::popClipRect(const glm::mat4& transform, VertexArrayPtr pVA)
{
    ScopeTimer timer(PopClipRectProfilingZone);
    m_ClipLevel--;
    clip(transform, pVA, GL_DECR);
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

bool Canvas::operator ==(const Canvas& other) const
{
    return this == &other;
}

bool Canvas::operator !=(const Canvas& other) const
{
    return this != &other;
}

long Canvas::getHash() const
{
    return long(this);
}

CanvasPtr Canvas::getActive()
{
    return s_pActiveCanvas;
}

Player* Canvas::getPlayer() const
{
    return m_pPlayer;
}

vector<NodeWeakPtr> Canvas::getElementsByPos(const glm::vec2& pos) const
{
    vector<NodeWeakPtr> elements;
    m_pRootNode->getElementsByPos(pos, elements);
    return elements;
}

static ProfilingZoneID PreRenderProfilingZone("PreRender");

void Canvas::render(IntPoint windowSize, bool bUpsideDown, FBOPtr pFBO,
        ProfilingZoneID& renderProfilingZone)
{
    {
        ScopeTimer Timer(PreRenderProfilingZone);
        m_pRootNode->preRender();
    }
    if (pFBO) {
        pFBO->activate();
    } else {
        glproc::BindFramebuffer(GL_FRAMEBUFFER_EXT, 0);
        GLContext::getCurrent()->checkError("Canvas::render: BindFramebuffer()");
    }
    if (m_MultiSampleSamples > 1) {
        glEnable(GL_MULTISAMPLE);
        GLContext::getCurrent()->checkError( 
                "Canvas::render: glEnable(GL_MULTISAMPLE)");
    } else {
        glDisable(GL_MULTISAMPLE);
        GLContext::getCurrent()->checkError(
                "Canvas::render: glDisable(GL_MULTISAMPLE)");
    }
    clearGLBuffers(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, windowSize.x, windowSize.y);
    GLContext::getCurrent()->checkError("Canvas::render: glViewport()");
    glMatrixMode(GL_PROJECTION);
    GLContext::getCurrent()->checkError("Canvas::render: glMatrixMode()");
    glLoadIdentity();
    GLContext::getCurrent()->checkError("Canvas::render: glLoadIdentity()");
    glm::vec2 size = m_pRootNode->getSize();
    if (bUpsideDown) {
        gluOrtho2D(0, size.x, 0, size.y);
    } else {
        gluOrtho2D(0, size.x, size.y, 0);
    }
    GLContext::getCurrent()->checkError("Canvas::render: gluOrtho2D()");
    
    glMatrixMode(GL_MODELVIEW);
    {
        ScopeTimer Timer(renderProfilingZone);
        m_pRootNode->maybeRender();

        renderOutlines();
    }
}

void Canvas::renderOutlines()
{
    GLContext* pContext = GLContext::getCurrent();
    VertexArrayPtr pVA(new VertexArray);
    pContext->setBlendMode(GLContext::BLEND_BLEND, false);
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(glm::value_ptr(glm::mat4(1.0)));
    m_pRootNode->renderOutlines(pVA, Pixel32(0,0,0,0));
    StandardShaderPtr pShader = GLContext::getCurrent()->getStandardShader();
    pShader->setUntextured();
    pShader->activate();
    if (pVA->getCurVert() != 0) {
        pVA->update();
        pContext->enableGLColorArray(true);
        pVA->draw();
    }
}

void Canvas::clip(const glm::mat4& transform, VertexArrayPtr pVA, GLenum stencilOp)
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
    pShader->activate();
    glLoadMatrixf(glm::value_ptr(transform));
    pVA->draw();

    // Set stencil test to only let
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
