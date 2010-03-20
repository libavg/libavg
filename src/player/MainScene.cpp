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

#include "MainScene.h"

#include "Player.h"
#include "SDLDisplayEngine.h"
#include "AVGNode.h"
#include "TestHelper.h"

#include "../base/Exception.h"
#include "../base/ScopeTimer.h"
#include "../base/Logger.h"

#include <vector>

using namespace boost;
using namespace std;

namespace avg {
    
MainScene::MainScene(Player * pPlayer, NodePtr pRootNode)
    : Scene(pPlayer, pRootNode),
      m_pEventDispatcher(new EventDispatcher)
{
    if (!dynamic_pointer_cast<AVGNode>(pRootNode)) {
        throw (Exception(AVG_ERR_XML_PARSE, 
                "Root node of an avg tree needs to be an <avg> node."));
    }
}

MainScene::~MainScene()
{
    m_MouseState = MouseState();
}

void MainScene::initPlayback(DisplayEngine* pDisplayEngine, AudioEngine* pAudioEngine,
        TestHelper* pTestHelper)
{
    Scene::initPlayback(pDisplayEngine, pAudioEngine, pTestHelper);
    m_pEventDispatcher->addSource(getDisplayEngine());
    m_pEventDispatcher->addSource(pTestHelper);
    m_pEventDispatcher->addSink(this);

}

void MainScene::addEventSource(IEventSource* pSource)
{
    m_pEventDispatcher->addSource(pSource);
}

MouseEventPtr MainScene::getMouseState() const
{
    return m_MouseState.getLastEvent();
}


AVGNodePtr MainScene::getRootNode() const
{
    return dynamic_pointer_cast<AVGNode>(Scene::getRootNode());
}

static ProfilingZone EventsProfilingZone("Dispatch events");

void MainScene::doFrame(bool bPythonAvailable)
{
    {
        ScopeTimer Timer(EventsProfilingZone);
        m_pEventDispatcher->dispatch();
        sendFakeEvents();
    }
    Scene::doFrame(bPythonAvailable);
}

bool MainScene::handleEvent(EventPtr pEvent)
{
    AVG_ASSERT(pEvent);
  
    PyObject * pEventHook = getPlayer()->getEventHook();
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
        getRootNode()->handleEvent(pKeyEvent);
        if (getPlayer()->getStopOnEscape() && pEvent->getType() == Event::KEYDOWN
                && pKeyEvent->getKeyCode() == avg::key::KEY_ESCAPE)
        {
            getPlayer()->stop();
        }
    } else {
        switch(pEvent->getType()){
            case Event::QUIT:
                getPlayer()->stop();
                break;
            default:
                AVG_TRACE(Logger::ERROR, "Unknown event type in Player::handleEvent.");
                break;
        }
    // Don't pass on any events.
    }
    return true; 
}

BitmapPtr MainScene::screenshot() const
{
    if (!getDisplayEngine()) {
        throw(Exception(AVG_ERR_UNSUPPORTED, 
                "MainScene::screenshot(): Scene is not being rendered. No screenshot available."));
    }
    return getDisplayEngine()->screenshot();
}

void MainScene::render()
{
    getDisplayEngine()->render(getRootNode(), false);
}

void MainScene::sendFakeEvents()
{
    std::map<int, CursorStatePtr>::iterator it;
    for (it=m_pLastCursorStates.begin(); it != m_pLastCursorStates.end(); ++it) {
        CursorStatePtr state = it->second;
        handleCursorEvent(state->getLastEvent(), true);
    }
}

void MainScene::sendOver(const CursorEventPtr pOtherEvent, Event::Type Type, 
        NodePtr pNode)
{
    if (pNode) {
        EventPtr pNewEvent = pOtherEvent->cloneAs(Type);
        pNewEvent->setElement(pNode);
        m_pEventDispatcher->sendEvent(pNewEvent);
    }
}

void MainScene::handleCursorEvent(CursorEventPtr pEvent, bool bOnlyCheckCursorOver)
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

}
