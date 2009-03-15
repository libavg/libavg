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

#include "DivNode.h"
#include "SDLDisplayEngine.h"
#include "Player.h"
#include "NodeDefinition.h"

#include "../base/Point.h"
#include "../base/Exception.h"
#include "../base/Logger.h"
#include "../base/StringHelper.h"
#include "../base/FileHelper.h"
#include "../base/MathHelper.h"

#include <iostream>
#include <sstream>

using namespace std;
using namespace boost;

namespace avg {

NodeDefinition DivNode::createDefinition()
{
    string sChildArray[] = {"image", "div", "canvas", "words", "video", "camera", 
            "panoimage", "sound", "line", "rect", "curve", "polyline", "polygon",
            "circle"};
    vector<string> sChildren = vectorFromCArray(
            sizeof(sChildArray) / sizeof(*sChildArray), sChildArray);
    return NodeDefinition("div", Node::buildNode<DivNode>)
        .extendDefinition(AreaNode::createDefinition())
        .addChildren(sChildren)
        .addArg(Arg<bool>("crop", true, false, offsetof(DivNode, m_bCrop)))
        .addArg(Arg<string>("mediadir", "", false, offsetof(DivNode, m_sMediaDir)));
}

DivNode::DivNode(const ArgList& Args, bool)
{
    Args.setMembers(this);
}

DivNode::~DivNode()
{
}

void DivNode::setRenderingEngines(DisplayEngine * pDisplayEngine, 
        AudioEngine * pAudioEngine)
{
    AreaNode::setRenderingEngines(pDisplayEngine, pAudioEngine);
    for  (int i = 0; i<(int)m_Children.size(); ++i) {
        m_Children[i]->setRenderingEngines(pDisplayEngine, pAudioEngine);
    }
}

void DivNode::connect()
{
    AreaNode::connect();
    for (int i = 0; i< (int)m_Children.size(); ++i) {
        m_Children[i]->connect();
    }
}

void DivNode::disconnect()
{
    for  (int i = 0; i< (int)m_Children.size(); ++i) {
        m_Children[i]->disconnect();
    }
    AreaNode::disconnect();
}

bool DivNode::getCrop() const
{
    return m_bCrop;
}

void DivNode::setCrop(bool bCrop)
{
    m_bCrop = bCrop;
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

int DivNode::getNumChildren()
{
    return int(m_Children.size());
}

const NodePtr& DivNode::getChild(unsigned i)
{
    if (i >= m_Children.size()) {
        stringstream s;
        s << "Index " << i << " is out of range in DivNode::getChild()";
        throw(Exception(AVG_ERR_OUT_OF_RANGE, s.str()));
    }
    return m_Children[i];
}

void DivNode::appendChild(NodePtr pNewNode)
{
    insertChild(pNewNode, unsigned(m_Children.size()));
}

void DivNode::insertChildBefore(NodePtr pNewNode, NodePtr pOldChild)
{
    if (!pOldChild) {
        throw Exception(AVG_ERR_NO_NODE,
                getID()+"::insertChildBefore called without a node.");
    }
    unsigned i = indexOf(pOldChild);
    insertChild(pNewNode, i);
}


void DivNode::insertChild(NodePtr pNewNode, unsigned i)
{
    if (!pNewNode) {
        throw Exception(AVG_ERR_NO_NODE,
                getID()+"::insertChild called without a node.");
    }
    if (!isChildTypeAllowed(pNewNode->getTypeStr())) {
        throw(Exception(AVG_ERR_ALREADY_CONNECTED,
                "Can't insert a node of type "+pNewNode->getTypeStr()+
                " into a node of type "+getTypeStr()+"."));

    }
    if (pNewNode->getState() == NS_CONNECTED || pNewNode->getState() == NS_CANRENDER) 
    {
        throw(Exception(AVG_ERR_ALREADY_CONNECTED,
                "Can't connect node with id "+pNewNode->getID()+
                ": already connected."));
    }
    if (i>m_Children.size()) {
        throw(Exception(AVG_ERR_OUT_OF_RANGE,
                pNewNode->getID()+"::insertChild: index out of bounds."));
    }
    std::vector<NodePtr>::iterator Pos = m_Children.begin()+i;
    if (getState() == NS_CONNECTED || getState() == NS_CANRENDER) {
        Player::get()->registerNode(pNewNode);
    }
    m_Children.insert(Pos, pNewNode);
    DivNodePtr Ptr = boost::dynamic_pointer_cast<DivNode>(getThis());           
    pNewNode->setParent(Ptr, getState());
    if (getState() == NS_CANRENDER) {
        pNewNode->setRenderingEngines(getDisplayEngine(), getAudioEngine());
    }
}

void DivNode::removeChild(NodePtr pNode)
{
    int i = indexOf(pNode);
    pNode->removeParent();
    m_Children.erase(m_Children.begin()+i);
}

void DivNode::removeChild(unsigned i)
{
    if (i>m_Children.size()-1) {
        throw(Exception(AVG_ERR_OUT_OF_RANGE,
                getID()+"::removeChild: index "+toString(i)+" out of bounds."));
    }
    NodePtr pNode = getChild(i);
    pNode->removeParent();
    m_Children.erase(m_Children.begin()+i);
}

void DivNode::reorderChild(NodePtr pNode, unsigned j)
{
    if (j > m_Children.size()-1) {
        throw(Exception(AVG_ERR_OUT_OF_RANGE,
                getID()+"::reorderChild: index "+toString(j)+" out of bounds."));
    }
    int i = indexOf(pNode);
    m_Children.erase(m_Children.begin()+i);
    std::vector<NodePtr>::iterator Pos = m_Children.begin()+j;
    m_Children.insert(Pos, pNode);
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
    if (!pChild) {
        throw Exception(AVG_ERR_NO_NODE,
                getID()+"::indexOf called without a node.");
    }
    for  (int i = 0; i< (int)m_Children.size(); ++i) {
        if (m_Children[i] == pChild) {
            return i;
        }
    }
    throw(Exception(AVG_ERR_OUT_OF_RANGE,
            "indexOf: node '"+pChild->getID()+"' is not a child of node '"
            +getID()+"'"));
}

AreaNodePtr DivNode::getElementByPos(const DPoint & pos)
{
    if (pos.x >= 0 && pos.y >= 0 && pos.x < getSize().x && pos.y < getSize().y &&
            reactsToMouseEvents())
    {
        for (int i=getNumChildren()-1; i>=0; i--) {
            AreaNodePtr pCurChild = dynamic_pointer_cast<AreaNode>(getChild(i));
            if (pCurChild) {
                DPoint relPos = pCurChild->toLocal(pos);
                AreaNodePtr pFoundNode = pCurChild->getElementByPos(relPos);
                if (pFoundNode) {
                    return pFoundNode;
                }
            }
        }
        // Pos isn't in any of the children.
        if (getSize() != DPoint(10000, 10000)) {
            // Explicit width/height given for div.
            return dynamic_pointer_cast<AreaNode>(getThis());
        } else {
            // Explicit width/height not given: div itself doesn't react.
            return AreaNodePtr();
        }
    } else { 
        return AreaNodePtr();
    }
}

void DivNode::preRender()
{
    Node::preRender();
    for (int i=0; i<getNumChildren(); i++) {
        getChild(i)->preRender();
    }
}

void DivNode::render(const DRect& rect)
{
    DPoint Viewport = getSize();
    if (getCrop()) {
        DRect ClipRect(0, 0, Viewport.x, Viewport.y);
        getDisplayEngine()->pushClipRect(ClipRect);
    }
    for (int i=0; i<getNumChildren(); i++) {
        getDisplayEngine()->pushShader();
        getChild(i)->maybeRender(rect);
        getDisplayEngine()->popShader();
    }
    if (getCrop()) {
        getDisplayEngine()->popClipRect();
    }
}

string DivNode::getEffectiveMediaDir()
{
    string sMediaDir = m_sMediaDir;
    if (!isAbsPath(sMediaDir)) {
        if (getParent()) {
            sMediaDir = getParent()->getEffectiveMediaDir()+m_sMediaDir;
        } else {
            sMediaDir = Player::get()->getRootMediaDir()+m_sMediaDir;
        }
    }
    if (sMediaDir[sMediaDir.length()-1] != '/') {
        sMediaDir += '/';
    }
    return sMediaDir;
}

void DivNode::checkReload()
{
    for(int i=0; i<getNumChildren(); ++i) {
        getChild(i)->checkReload();
    }
}

string DivNode::dump(int indent)
{
    string dumpStr = AreaNode::dump () + "\n";
    vector<NodePtr>::iterator it;
    for (it=m_Children.begin(); it<m_Children.end(); it++) {
        dumpStr += (*it)->dump(indent+2)+"\n";
    }
    return dumpStr;
}

IntPoint DivNode::getMediaSize()
{
    return IntPoint(10000,10000);
}
 
bool DivNode::isChildTypeAllowed(const string& sType)
{
    return getDefinition()->isChildAllowed(sType);
}

}
