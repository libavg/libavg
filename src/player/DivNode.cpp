//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2006 Ulrich von Zadow
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

#include "DivNode.h"
#include "DisplayEngine.h"
#include "Player.h"
#include "NodeDefinition.h"

#include "../base/Exception.h"
#include "../base/Logger.h"
#include "../base/XMLHelper.h"

#include <iostream>
#include <sstream>

using namespace std;

namespace avg {

NodeDefinition DivNode::getNodeDefinition()
{
    return NodeDefinition("div", Node::buildNode<DivNode>)
        .extendDefinition(Node::getNodeDefinition())
        .setGroupNode()
        .addArg(Arg<string>("mediadir", "", false, offsetof(DivNode, m_sMediaDir)));
}

DivNode::DivNode (const ArgList& Args, Player * pPlayer)
    : Node(pPlayer)
{
    Args.setMembers(this);
}

DivNode::~DivNode()
{
}

void DivNode::setRenderingEngines(DisplayEngine * pDisplayEngine, AudioEngine * pAudioEngine)
{
    Node::setRenderingEngines(pDisplayEngine, pAudioEngine);
    for  (int i = 0; i< (int)m_Children.size(); ++i) {
        m_Children[i]->setRenderingEngines(pDisplayEngine, pAudioEngine);
    }
}

void DivNode::disconnect()
{
    for  (int i = 0; i< (int)m_Children.size(); ++i) {
        m_Children[i]->disconnect();
    }
    Node::disconnect();
}

const string& DivNode::getMediaDir() const
{
    return m_sMediaDir;
}

void DivNode::setMediaDir(const string& sMediaDir)
{
    m_sMediaDir = sMediaDir;
    checkReload();
}

int DivNode::getNumChildren ()
{
    return int(m_Children.size());
}

NodePtr DivNode::getChild (unsigned i)
{
    if (i >= m_Children.size()) {
        stringstream s;
        s << "Index " << i << " is out of range in DivNode::getChild()";
        throw(Exception(AVG_ERR_OUT_OF_RANGE, s.str()));
    }
    return m_Children[i];
}

void DivNode::appendChild (NodePtr pNewNode)
{
    insertChild(pNewNode, unsigned(m_Children.size()));
}

void DivNode::insertChild(NodePtr pNewNode, unsigned i)
{
    if (!pNewNode) {
        throw Exception(AVG_ERR_NO_NODE,
                "insertChild called without a node.");
    }
    if (pNewNode->getState() == NS_CONNECTED) {
        throw(Exception(AVG_ERR_ALREADY_CONNECTED,
                "Can't connect node with id "+pNewNode->getID()+
                ": already connected."));
    }
    if (i>m_Children.size()) {
        throw(Exception(AVG_ERR_OUT_OF_RANGE,
                pNewNode->getID()+"::insertChild: index out of bounds."));
    }
    std::vector<NodePtr>::iterator Pos = m_Children.begin()+i;
    if (getState() == NS_CONNECTED) {
        getPlayer()->registerNode(pNewNode);
    }
    m_Children.insert(Pos, pNewNode);
    DivNodePtr Ptr = boost::dynamic_pointer_cast<DivNode>(getThis());           
    pNewNode->setParent(Ptr);
    if (isDisplayAvailable()) {
        pNewNode->setRenderingEngines(getDisplayEngine(), getAudioEngine());
    }
}

void DivNode::removeChild (unsigned i)
{
    NodePtr pNode = getChild(i);
    pNode->setParent(DivNodePtr());
    pNode->disconnect();
    m_Children.erase(m_Children.begin()+i);
}

void DivNode::reorderChild(unsigned i, unsigned j)
{
    if (i>m_Children.size()-1 || j > m_Children.size()-1) {
        throw(Exception(AVG_ERR_OUT_OF_RANGE,
                getID()+"::reorderChild: index out of bounds."));
    }
    NodePtr pNode = getChild(i);
    m_Children.erase(m_Children.begin()+i);
    std::vector<NodePtr>::iterator Pos = m_Children.begin()+j;
    m_Children.insert(Pos, pNode);
}

int DivNode::indexOf(NodePtr pChild)
{
    for  (int i = 0; i< (int)m_Children.size(); ++i) {
        if (m_Children[i] == pChild) {
            return i;
        }
    }
    return -1;
}

NodePtr DivNode::getElementByPos (const DPoint & pos)
{
    DPoint relPos = toLocal(pos);
    if (relPos.x >= 0 && relPos.y >= 0 && 
            relPos.x < getRelSize().x && relPos.y < getRelSize().y &&
            reactsToMouseEvents())
    {
        for (int i=getNumChildren()-1; i>=0; i--) {
            NodePtr pFoundNode = getChild(i)->getElementByPos(relPos);
            if (pFoundNode) {
                return pFoundNode;
            }
        }
        // Pos isn't in any of the children.
        if (getRelSize() != DPoint(10000, 10000)) {
            // Explicit width/height given for div.
            return getThis(); 
        } else {
            // Explicit width/height not given: div itself doesn't react.
            return NodePtr();
        }
    } else { 
        return NodePtr();
    }
}

void DivNode::preRender()
{
    for (int i=0; i<getNumChildren(); i++) {
        getChild(i)->preRender();
    }
}

void DivNode::render(const DRect& rect)
{
    DPoint Viewport = getRelSize();
    DRect ClipRect(0, 0, Viewport.x, Viewport.y);
    getDisplayEngine()->pushClipRect(ClipRect);
    for (int i=0; i<getNumChildren(); i++) {
        getChild(i)->maybeRender(rect);
    }
    getDisplayEngine()->popClipRect();
}

string DivNode::getTypeStr ()
{
    return "DivNode";
}

string DivNode::dump (int indent)
{
    string dumpStr = Node::dump () + "\n";
    vector<NodePtr>::iterator it;
    for (it=m_Children.begin(); it<m_Children.end(); it++) {
        dumpStr += (*it)->dump(indent+2)+"\n";
    }
    return dumpStr;
}

DPoint DivNode::getPreferredMediaSize()
{
    return DPoint(10000,10000);
}

string DivNode::getEffectiveMediaDir()
{
    string sMediaDir;
    if (getParent()) {
        sMediaDir = getParent()->getEffectiveMediaDir()+m_sMediaDir;
    } else {
        sMediaDir = getPlayer()->getRootMediaDir()+m_sMediaDir;
    }
    if (sMediaDir[sMediaDir.length()-1] != '/') {
        sMediaDir += '/';
    }
    return sMediaDir;
}

void DivNode::checkReload()
{
    vector<NodePtr>::iterator it;
    for (it=m_Children.begin(); it<m_Children.end(); it++) {
        (*it)->checkReload();
    }
}

}
