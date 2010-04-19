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

#include "Canvas.h"

#include "Player.h"
#include "AVGNode.h"
#include "SDLDisplayEngine.h"
#include "Shape.h"

#include "../base/Exception.h"
#include "../base/Logger.h"
#include "../base/ScopeTimer.h"

#include <iostream>

using namespace std;
using namespace boost;

namespace avg {

Canvas::Canvas(Player * pPlayer)
    : m_pPlayer(pPlayer),
      m_pDisplayEngine(0),
      m_PlaybackEndSignal(&IPlaybackEndListener::onPlaybackEnd),
      m_FrameEndSignal(&IFrameEndListener::onFrameEnd),
      m_PreRenderSignal(&IPreRenderListener::onPreRender)
{
}

Canvas::~Canvas()
{
}

void Canvas::setRoot(NodePtr pRootNode)
{
    assert(!m_pRootNode);
    m_pRootNode = dynamic_pointer_cast<CanvasNode>(pRootNode);
    m_pRootNode->setParent(DivNodeWeakPtr(), VisibleNode::NS_CONNECTED,
            shared_from_this());
    registerNode(m_pRootNode);
}

void Canvas::initPlayback(SDLDisplayEngine* pDisplayEngine, AudioEngine* pAudioEngine,
        int multiSampleSamples)
{
    m_pDisplayEngine = pDisplayEngine;
    m_pRootNode->setRenderingEngines(m_pDisplayEngine, pAudioEngine);
    m_MultiSampleSamples = multiSampleSamples;
}

void Canvas::stopPlayback()
{
    m_PlaybackEndSignal.emit();
    m_pRootNode->disconnect(true);
    m_pRootNode = CanvasNodePtr();
    m_IDMap.clear();
}

VisibleNodePtr Canvas::getElementByID(const std::string& id)
{
    if (m_IDMap.find(id) != m_IDMap.end()) {
        return m_IDMap.find(id)->second;
    } else {
        AVG_TRACE(Logger::WARNING, "getElementByID(\"" << id << "\") failed.");
        return VisibleNodePtr();
    }
}

void Canvas::registerNode(VisibleNodePtr pNode)
{
    addNodeID(pNode);    
    DivNodePtr pDivNode = boost::dynamic_pointer_cast<DivNode>(pNode);
    if (pDivNode) {
        for (unsigned i=0; i<pDivNode->getNumChildren(); i++) {
            registerNode(pDivNode->getVChild(i));
        }
    }
}

void Canvas::addNodeID(VisibleNodePtr pNode)
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

void Canvas::removeNodeID(const std::string& id)
{
    if (id != "") {
        std::map<std::string, VisibleNodePtr>::iterator it;
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

static ProfilingZone PreRenderProfilingZone("PreRender");
static ProfilingZone RenderProfilingZone("Render");
static ProfilingZone FrameEndProfilingZone("OnFrameEnd");

void Canvas::doFrame(bool bPythonAvailable)
{
    {
        ScopeTimer Timer(PreRenderProfilingZone);
        m_PreRenderSignal.emit();
    }
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
    {
        ScopeTimer Timer(FrameEndProfilingZone);
        m_FrameEndSignal.emit();
    }
}

IntPoint Canvas::getSize() const
{
    return IntPoint(m_pRootNode->getSize());
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

Player* Canvas::getPlayer() const
{
    return m_pPlayer;
}

SDLDisplayEngine* Canvas::getDisplayEngine() const
{
    return m_pDisplayEngine;
}

vector<VisibleNodeWeakPtr> Canvas::getElementsByPos(const DPoint& pos) const
{
    vector<VisibleNodeWeakPtr> Elements;
    VisibleNodePtr pNode = m_pRootNode->getElementByPos(pos);
    while (pNode) {
        Elements.push_back(pNode);
        pNode = pNode->getDivParent();
    }
    return Elements;
}


void Canvas::render(IntPoint windowSize, bool bUpsideDown,
        ProfilingZone& renderProfilingZone)
{
    m_pRootNode->preRender();
    if (m_MultiSampleSamples > 1) {
        glEnable(GL_MULTISAMPLE);
        OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
                "SDLDisplayEngine::render: glEnable(GL_MULTISAMPLE)");
    } else {
        glDisable(GL_MULTISAMPLE);
        OGLErrorCheck(AVG_ERR_VIDEO_GENERAL,
                "SDLDisplayEngine::render: glDisable(GL_MULTISAMPLE)");
    }
    glClearColor(0.0, 0.0, 0.0, 0.0); 
    glClear(GL_COLOR_BUFFER_BIT);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            "SDLDisplayEngine::render::glClear(GL_COLOR_BUFFER_BIT)");
    glStencilMask(~0);
    glClearStencil(0);
    glClear(GL_STENCIL_BUFFER_BIT);
    glStencilMask(0);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            "SDLDisplayEngine::render::glClear(GL_STENCIL_BUFFER_BIT)");
    glClear(GL_DEPTH_BUFFER_BIT);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            "SDLDisplayEngine::render::glClear(GL_DEPTH_BUFFER_BIT)");
    glViewport(0, 0, windowSize.x, windowSize.y);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "SDLDisplayEngine::render: glViewport()");
    glMatrixMode(GL_PROJECTION);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "SDLDisplayEngine::render: glMatrixMode()");
    glLoadIdentity();
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "SDLDisplayEngine::render: glLoadIdentity()");
    IntPoint size = IntPoint(m_pRootNode->getSize());
    if (bUpsideDown) {
        gluOrtho2D(0, size.x, 0, size.y);
    } else {
        gluOrtho2D(0, size.x, size.y, 0);
    }
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "SDLDisplayEngine::render: gluOrtho2D()");
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE); 
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "SDLDisplayEngine::render: glTexEnvf()");
    glproc::BlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, 
            GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            "SDLDisplayEngine::render: glBlendFuncSeparate()");
    
    const DRect rc(0,0, size.x, size.y);
    glMatrixMode(GL_MODELVIEW);
    {
        ScopeTimer Timer(renderProfilingZone);
        m_pRootNode->maybeRender(rc);

        Shape * pShape = new Shape(MaterialInfo(GL_REPEAT, GL_CLAMP_TO_EDGE, false));
        pShape->moveToGPU(m_pDisplayEngine);
        VertexArrayPtr pVA = pShape->getVertexArray();
        m_pRootNode->renderOutlines(pVA, Pixel32(0,0,0,0));
        if (pVA->getCurVert() != 0) {
            pVA->update();
            pShape->draw();
        }
        delete pShape;
    }
}

}
