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

#include "Scene.h"

#include "Player.h"
#include "AVGNode.h"
#include "TestHelper.h"
#include "SDLDisplayEngine.h"

#include "../base/Exception.h"
#include "../base/Logger.h"
#include "../base/ScopeTimer.h"

#include <iostream>

using namespace std;
using namespace boost;

namespace avg {

Scene::Scene(Player * pPlayer, NodePtr pRootNode)
    : m_pPlayer(pPlayer),
      m_pRootNode(dynamic_pointer_cast<SceneNode>(pRootNode)),
      m_pDisplayEngine(0),
      m_pEventDispatcher(new EventDispatcher),
      m_pEventCaptureNode(),
      m_PlaybackEndSignal(&IPlaybackEndListener::onPlaybackEnd),
      m_FrameEndSignal(&IFrameEndListener::onFrameEnd),
      m_PreRenderSignal(&IPreRenderListener::onPreRender)
{
    m_pRootNode->setParent(DivNodeWeakPtr(), Node::NS_CONNECTED, this);
    registerNode(m_pRootNode);
}

Scene::~Scene()
{
    m_PlaybackEndSignal.emit();
    m_pRootNode->disconnect(true);
    m_pRootNode = SceneNodePtr();
    m_IDMap.clear();
    m_MouseState = MouseState();
}

void Scene::initPlayback(DisplayEngine* pDisplayEngine, AudioEngine* pAudioEngine,
        TestHelper* pTestHelper)
{
    m_pDisplayEngine = pDisplayEngine;
    m_pRootNode->setRenderingEngines(m_pDisplayEngine, pAudioEngine);
    m_pEventDispatcher->addSource(dynamic_cast<SDLDisplayEngine *>(m_pDisplayEngine));
    m_pEventDispatcher->addSource(pTestHelper);
    m_pEventDispatcher->addSink(this);

}

void Scene::addEventSource(IEventSource* pSource)
{
    m_pEventDispatcher->addSource(pSource);
}

MouseEventPtr Scene::getMouseState() const
{
    return m_MouseState.getLastEvent();
}

void Scene::setEventCapture(NodePtr pNode, int cursorID=MOUSECURSORID)
{
    std::map<int, NodeWeakPtr>::iterator it = m_pEventCaptureNode.find(cursorID);
    if (it!=m_pEventCaptureNode.end()&&!it->second.expired()) {
        throw Exception(AVG_ERR_INVALID_CAPTURE, "setEventCapture called for '"
                + pNode->getID() + "', but cursor already captured by '"
                + it->second.lock()->getID() + "'.");
    } else {
        m_pEventCaptureNode[cursorID] = pNode;
    }
}

void Scene::releaseEventCapture(int cursorID)
{
    std::map<int, NodeWeakPtr>::iterator it = m_pEventCaptureNode.find(cursorID);
    if(it==m_pEventCaptureNode.end()||(it->second.expired()) ) {
        throw Exception(AVG_ERR_INVALID_CAPTURE,
                "releaseEventCapture called, but cursor not captured.");
    } else {
        m_pEventCaptureNode.erase(cursorID);
    }
}


NodePtr Scene::getElementByID(const std::string& id)
{
    if (m_IDMap.find(id) != m_IDMap.end()) {
        return m_IDMap.find(id)->second;
    } else {
        AVG_TRACE(Logger::WARNING, "getElementByID(\"" << id << "\") failed.");
        return NodePtr();
    }
}

void Scene::registerNode(NodePtr pNode)
{
    addNodeID(pNode);    
    DivNodePtr pDivNode = boost::dynamic_pointer_cast<DivNode>(pNode);
    if (pDivNode) {
        for (int i=0; i<pDivNode->getNumChildren(); i++) {
            registerNode(pDivNode->getChild(i));
        }
    }
}

void Scene::addNodeID(NodePtr pNode)
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

void Scene::removeNodeID(const std::string& id)
{
    if (id != "") {
        std::map<std::string, NodePtr>::iterator it;
        it = m_IDMap.find(id);
        if (it != m_IDMap.end()) {
            m_IDMap.erase(it);
        } else {
            cerr << "removeNodeID(\"" << id << "\") failed." << endl;
            AVG_ASSERT(false);
        }
    }
}

SceneNodePtr Scene::getRootNode() const
{
    return m_pRootNode;
}

static ProfilingZone EventsProfilingZone("Dispatch events");
static ProfilingZone PreRenderProfilingZone("PreRender");
static ProfilingZone RenderProfilingZone("Render");
static ProfilingZone FrameEndProfilingZone("OnFrameEnd");

void Scene::doFrame(bool bPythonAvailable)
{
    {
        ScopeTimer Timer(EventsProfilingZone);
        m_pEventDispatcher->dispatch();
        sendFakeEvents();
    }
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

bool Scene::handleEvent(EventPtr pEvent)
{
    AVG_ASSERT(pEvent);
  
    PyObject * pEventHook = m_pPlayer->getEventHook();
    if (pEventHook != Py_None) {
        // If the catchall returns true, stop processing the event
        if (boost::python::call<bool>(pEventHook, pEvent)) {
            return true;
        }
    }

    if (MouseEventPtr pMouseEvent = boost::dynamic_pointer_cast<MouseEvent>(pEvent)) {
        m_MouseState.setEvent(pMouseEvent);
        pMouseEvent->setLastDownPos(m_MouseState.getLastDownPos());
    }
    
    if (CursorEventPtr pCursorEvent = boost::dynamic_pointer_cast<CursorEvent>(pEvent)) {
        if (pEvent->getType() == Event::CURSOROUT || 
                pEvent->getType() == Event::CURSOROVER)
        {
            pEvent->trace();
            pEvent->getElement()->handleEvent(pEvent);
        } else {
            handleCursorEvent(pCursorEvent);
        }
    }
    else if (KeyEventPtr pKeyEvent = boost::dynamic_pointer_cast<KeyEvent>(pEvent))
    {
        pEvent->trace();
        m_pRootNode->handleEvent(pKeyEvent);
        if (m_pPlayer->getStopOnEscape() && pEvent->getType() == Event::KEYDOWN
                && pKeyEvent->getKeyCode() == avg::key::KEY_ESCAPE)
        {
            m_pPlayer->stop();
        }
    } else {
        switch(pEvent->getType()){
            case Event::QUIT:
                m_pPlayer->stop();
                break;
            default:
                AVG_TRACE(Logger::ERROR, "Unknown event type in Player::handleEvent.");
                break;
        }
    // Don't pass on any events.
    }
    return true; 
}

IntPoint Scene::getSize() const
{
    return IntPoint(m_pRootNode->getSize());
}

void Scene::registerPlaybackEndListener(IPlaybackEndListener* pListener)
{
    m_PlaybackEndSignal.connect(pListener);
}

void Scene::unregisterPlaybackEndListener(IPlaybackEndListener* pListener)
{
    m_PlaybackEndSignal.disconnect(pListener);
}

void Scene::registerFrameEndListener(IFrameEndListener* pListener)
{
    m_FrameEndSignal.connect(pListener);
}

void Scene::unregisterFrameEndListener(IFrameEndListener* pListener)
{
    m_FrameEndSignal.disconnect(pListener);
}

void Scene::registerPreRenderListener(IPreRenderListener* pListener)
{
    m_PreRenderSignal.connect(pListener);
}

void Scene::unregisterPreRenderListener(IPreRenderListener* pListener)
{
    m_PreRenderSignal.disconnect(pListener);
}

bool Scene::operator ==(const Scene& other) const
{
    return this == &other;
}

bool Scene::operator !=(const Scene& other) const
{
    return this != &other;
}

long Scene::getHash() const
{
    return long(this);
}


SDLDisplayEngine* Scene::getDisplayEngine() const
{
    return dynamic_cast<SDLDisplayEngine *>(m_pDisplayEngine);
}

void Scene::sendFakeEvents()
{
    std::map<int, CursorStatePtr>::iterator it;
    for (it=m_pLastCursorStates.begin(); it != m_pLastCursorStates.end(); ++it) {
        CursorStatePtr state = it->second;
        handleCursorEvent(state->getLastEvent(), true);
    }
}

void Scene::sendOver(const CursorEventPtr pOtherEvent, Event::Type Type, NodePtr pNode)
{
    if (pNode) {
        EventPtr pNewEvent = pOtherEvent->cloneAs(Type);
        pNewEvent->setElement(pNode);
        m_pEventDispatcher->sendEvent(pNewEvent);
    }
}

void Scene::handleCursorEvent(CursorEventPtr pEvent, bool bOnlyCheckCursorOver)
{
    DPoint pos(pEvent->getXPosition(), pEvent->getYPosition());
    int cursorID = pEvent->getCursorID();
    // Find all nodes under the cursor.
    vector<NodeWeakPtr> pCursorNodes = getElementsByPos(pos);

    // Determine the nodes the event should be sent to.
    vector<NodeWeakPtr> pDestNodes = pCursorNodes;
    bool bIsCapturing = false;
    if (m_pEventCaptureNode.find(cursorID) != m_pEventCaptureNode.end()) {
        NodeWeakPtr pEventCaptureNode = m_pEventCaptureNode[cursorID];
        if (pEventCaptureNode.expired()) {
            m_pEventCaptureNode.erase(cursorID);
        } else {
            pDestNodes = pEventCaptureNode.lock()->getParentChain();
        }
    } 

    vector<NodeWeakPtr> pLastCursorNodes;
    {
        map<int, CursorStatePtr>::iterator it;
        it = m_pLastCursorStates.find(cursorID);
        if (it != m_pLastCursorStates.end()) {
            pLastCursorNodes = it->second->getNodes();
        }
    }

    // Send out events.
    vector<NodeWeakPtr>::const_iterator itLast;
    vector<NodeWeakPtr>::iterator itCur;
    for (itLast = pLastCursorNodes.begin(); itLast != pLastCursorNodes.end(); ++itLast) {
        NodePtr pLastNode = itLast->lock();
        for (itCur = pCursorNodes.begin(); itCur != pCursorNodes.end(); ++itCur) {
            if (itCur->lock() == pLastNode) {
                break;
            }
        }
        if (itCur == pCursorNodes.end()) {
            if (!bIsCapturing || pLastNode == pDestNodes.begin()->lock()) {
                sendOver(pEvent, Event::CURSOROUT, pLastNode);
            }
        }
    } 

    // Send over events.
    for (itCur = pCursorNodes.begin(); itCur != pCursorNodes.end(); ++itCur) {
        NodePtr pCurNode = itCur->lock();
        for (itLast = pLastCursorNodes.begin(); itLast != pLastCursorNodes.end(); 
                ++itLast) 
        {
            if (itLast->lock() == pCurNode) {
                break;
            }
        }
        if (itLast == pLastCursorNodes.end()) {
            if (!bIsCapturing || pCurNode == pDestNodes.begin()->lock()) {
                sendOver(pEvent, Event::CURSOROVER, pCurNode);
            }
        }
    } 

    if (!bOnlyCheckCursorOver) {
        // Iterate through the nodes and send the event to all of them.
        vector<NodeWeakPtr>::iterator it;
        for (it = pDestNodes.begin(); it != pDestNodes.end(); ++it) {
            NodePtr pNode = (*it).lock();
            if (pNode) {
                CursorEventPtr pNodeEvent = boost::dynamic_pointer_cast<CursorEvent>(
                        pEvent->cloneAs(pEvent->getType()));
                pNodeEvent->setElement(pNode);
                if (pNodeEvent->getType() != Event::CURSORMOTION) {
                    pNodeEvent->trace();
                }
                if (pNode->handleEvent(pNodeEvent) == true) {
                    // stop bubbling
                    break;
                }
            }
        }
    }
    if (pEvent->getType() == Event::CURSORUP && pEvent->getSource() != Event::MOUSE) {
        // Cursor has disappeared: send out events.
        if (bIsCapturing) {
            NodePtr pNode = pDestNodes.begin()->lock();
            sendOver(pEvent, Event::CURSOROUT, pNode);
        } else {
            vector<NodeWeakPtr>::iterator it;
            for (it = pCursorNodes.begin(); it != pCursorNodes.end(); ++it) {
                NodePtr pNode = it->lock();
                sendOver(pEvent, Event::CURSOROUT, pNode);
            } 
        }
        m_pLastCursorStates.erase(cursorID);
    } else {
        // Update list of nodes under cursor
        if (m_pLastCursorStates.find(cursorID) != m_pLastCursorStates.end()) {
            m_pLastCursorStates[cursorID]->setInfo(pEvent, pCursorNodes);
        } else {
            m_pLastCursorStates[cursorID] =
                    CursorStatePtr(new CursorState(pEvent, pCursorNodes));
        }
    }
}

vector<NodeWeakPtr> Scene::getElementsByPos(const DPoint& pos) const
{
    vector<NodeWeakPtr> Elements;
    NodePtr pNode = m_pRootNode->getElementByPos(pos);
    while (pNode) {
        Elements.push_back(pNode);
        pNode = pNode->getParent();
    }
    return Elements;
}

}
