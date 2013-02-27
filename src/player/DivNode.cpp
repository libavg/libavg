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

#include "DivNode.h"
#include "Player.h"
#include "TypeDefinition.h"
#include "Canvas.h"

#include "../base/Exception.h"
#include "../base/Logger.h"
#include "../base/StringHelper.h"
#include "../base/FileHelper.h"
#include "../base/MathHelper.h"
#include "../base/ObjectCounter.h"

#include <iostream>
#include <sstream>
#include <limits>

using namespace std;
using namespace boost;

namespace avg {


void DivNode::registerType()
{
    string sChildArray[] = {"image", "div", "canvas", "words", "video", "camera", 
            "panoimage", "sound", "line", "rect", "curve", "polyline", "polygon",
            "circle", "mesh"};
    vector<string> sChildren = vectorFromCArray(
            sizeof(sChildArray) / sizeof(*sChildArray), sChildArray);
    TypeDefinition def = TypeDefinition("div", "areanode", 
            ExportedObject::buildObject<DivNode>)
        .addChildren(sChildren)
        .addArg(Arg<bool>("crop", false, false, offsetof(DivNode, m_bCrop)))
        .addArg(Arg<UTF8String>("mediadir", "", false, offsetof(DivNode, m_sMediaDir)));
    TypeRegistry::get()->registerType(def);
}

DivNode::DivNode(const ArgList& args)
{
    args.setMembers(this);
    ObjectCounter::get()->incRef(&typeid(*this));
}

DivNode::~DivNode()
{
    for (unsigned i = 0; i < getNumChildren(); ++i) {
        getChild(i)->removeParent();
    }
    ObjectCounter::get()->decRef(&typeid(*this));
}

void DivNode::connectDisplay()
{
    AreaNode::connectDisplay();
    for (unsigned i = 0; i < getNumChildren(); ++i) {
        getChild(i)->connectDisplay();
    }
}

void DivNode::connect(CanvasPtr pCanvas)
{
    AreaNode::connect(pCanvas);
    for (unsigned i = 0; i < getNumChildren(); ++i) {
        getChild(i)->connect(pCanvas);
    }
}

void DivNode::disconnect(bool bKill)
{
    for (unsigned i = 0; i < getNumChildren(); ++i) {
        getChild(i)->disconnect(bKill);
    }
    AreaNode::disconnect(bKill);
}

glm::vec2 DivNode::getPivot() const
{
    glm::vec2 pivot = AreaNode::getPivot();
    return pivot;
}

unsigned DivNode::getNumChildren()
{
    return m_Children.size();
}

const NodePtr& DivNode::getChild(unsigned i)
{
    if (i >= m_Children.size()) {
        stringstream s;
        s << "Index " << i << " is out of range in Node::getChild()";
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

void DivNode::insertChildAfter(NodePtr pNewNode, NodePtr pOldChild)
{
    if (!pOldChild) {
        throw Exception(AVG_ERR_NO_NODE,
                getID()+"::insertChildBefore called without a node.");
    }
    unsigned i = indexOf(pOldChild);
    insertChild(pNewNode, i+1);
}

void DivNode::insertChild(NodePtr pChild, unsigned i)
{
    if (!pChild) {
        throw Exception(AVG_ERR_NO_NODE,
                getID()+"::insertChild called without a node.");
    }
    if (pChild->getState() == NS_CONNECTED || pChild->getState() == NS_CANRENDER) {
        throw(Exception(AVG_ERR_ALREADY_CONNECTED,
                "Can't connect node with id "+pChild->getID()+
                ": already connected."));
    }
    if (getState() == NS_CONNECTED || getState() == NS_CANRENDER) {
        getCanvas()->registerNode(pChild);
    }
    pChild->checkSetParentError(this); 
    if (!isChildTypeAllowed(pChild->getTypeStr())) {
        throw(Exception(AVG_ERR_ALREADY_CONNECTED,
                "Can't insert a node of type "+pChild->getTypeStr()+
                " into a node of type "+getTypeStr()+"."));
    }
    if (i > m_Children.size()) {
        throw(Exception(AVG_ERR_OUT_OF_RANGE,
                pChild->getID()+"::insertChild: index out of bounds."));
    }
    std::vector<NodePtr>::iterator pos = m_Children.begin()+i;
    m_Children.insert(pos, pChild);
    try {
        pChild->setParent(this, getState(), getCanvas());
    } catch (Exception&) {
        m_Children.erase(m_Children.begin()+i);
        throw;
    }
    if (getState() == NS_CANRENDER) {
        pChild->connectDisplay();
    }
}

void DivNode::reorderChild(NodePtr pChild, unsigned j)
{
    if (j > m_Children.size()-1) {
        throw(Exception(AVG_ERR_OUT_OF_RANGE,
                getID()+"::reorderChild: index "+toString(j)+" out of bounds."));
    }
    int i = indexOf(pChild);
    m_Children.erase(m_Children.begin()+i);
    std::vector<NodePtr>::iterator pos = m_Children.begin()+j;
    m_Children.insert(pos, pChild);
}

void DivNode::reorderChild(unsigned i, unsigned j)
{
    if (i > m_Children.size()-1 || j > m_Children.size()-1) {
        throw(Exception(AVG_ERR_OUT_OF_RANGE,
                getID()+"::reorderChild: index out of bounds."));
    }
    NodePtr pChild = getChild(i);
    m_Children.erase(m_Children.begin()+i);
    std::vector<NodePtr>::iterator pos = m_Children.begin()+j;
    m_Children.insert(pos, pChild);
}

unsigned DivNode::indexOf(NodePtr pChild)
{
    if (!pChild) {
        throw Exception(AVG_ERR_NO_NODE,
                getID()+"::indexOf called without a node.");
    }
    for (unsigned i = 0; i < m_Children.size(); ++i) {
        if (m_Children[i] == pChild) {
            return i;
        }
    }
    throw(Exception(AVG_ERR_OUT_OF_RANGE,
            "indexOf: node '"+pChild->getID()+"' is not a child of node '"
            +getID()+"'"));
}

void DivNode::removeChild(NodePtr pChild)
{
    removeChild(pChild, false);
}

void DivNode::removeChild(unsigned i)
{
    removeChild(i, false);
}

void DivNode::removeChild(NodePtr pChild, bool bKill)
{
    pChild->removeParent();
    if (pChild->getState() != NS_UNCONNECTED) {
        pChild->disconnect(bKill);
    }
    unsigned i = indexOf(pChild);
    if (i > m_Children.size()-1) {
        throw(Exception(AVG_ERR_OUT_OF_RANGE,
                getID()+"::removeChild: index "+toString(i)+" out of bounds."));
    }
    m_Children.erase(m_Children.begin()+i);
}

void DivNode::removeChild(unsigned i, bool bKill)
{
    NodePtr pChild = getChild(i);
    removeChild(pChild, bKill);
}

bool DivNode::getCrop() const
{
    return m_bCrop;
}

void DivNode::setCrop(bool bCrop)
{
    m_bCrop = bCrop;
}

const UTF8String& DivNode::getMediaDir() const
{
    return m_sMediaDir;
}

void DivNode::setMediaDir(const UTF8String& sMediaDir)
{
//    avgDeprecationWarning("1.7", "DivNode.mediadir", "");
    m_sMediaDir = sMediaDir;
    checkReload();
}

void DivNode::getElementsByPos(const glm::vec2& pos, vector<NodePtr>& pElements)
{
    if (reactsToMouseEvents() &&
            ((getSize() == glm::vec2(0,0) ||
             (pos.x >= 0 && pos.y >= 0 && pos.x < getSize().x && pos.y < getSize().y))))
    {
        for (int i = getNumChildren()-1; i >= 0; i--) {
            NodePtr pCurChild = getChild(i);
            glm::vec2 relPos = pCurChild->toLocal(pos);
            pCurChild->getElementsByPos(relPos, pElements);
            if (!pElements.empty()) {
                pElements.push_back(getSharedThis());
                return;
            }
        }
        // pos isn't in any of the children.
        if (getSize() != glm::vec2(0,0)) {
            // Explicit width/height given for div - div reacts on its own.
            pElements.push_back(getSharedThis());
        }
    }
}

void DivNode::preRender(const VertexArrayPtr& pVA, bool bIsParentActive, 
        float parentEffectiveOpacity)
{
    Node::preRender(pVA, bIsParentActive, parentEffectiveOpacity);
    if (getCrop() && getSize() != glm::vec2(0,0)) {
        pVA->startSubVA(m_ClipVA);
        glm::vec2 viewport = getSize();
        m_ClipVA.appendPos(glm::vec2(0,0), glm::vec2(0,0), Pixel32(0,0,0,0));
        m_ClipVA.appendPos(glm::vec2(0,viewport.y), glm::vec2(0,0), Pixel32(0,0,0,0));
        m_ClipVA.appendPos(glm::vec2(viewport.x,0), glm::vec2(0,0), Pixel32(0,0,0,0));
        m_ClipVA.appendPos(viewport, glm::vec2(0,0), Pixel32(0,0,0,0));
        m_ClipVA.appendQuadIndexes(0, 1, 2, 3);
    }
    for (unsigned i = 0; i < getNumChildren(); i++) {
        getChild(i)->preRender(pVA, bIsParentActive, getEffectiveOpacity());
    }
}

void DivNode::render()
{
    const glm::mat4& transform = getTransform();
    if (getCrop() && getSize() != glm::vec2(0,0)) {
        getCanvas()->pushClipRect(transform, m_ClipVA);
    }
    for (unsigned i = 0; i < getNumChildren(); i++) {
        getChild(i)->maybeRender(transform);
    }
    if (getCrop() && getSize() != glm::vec2(0,0)) {
        getCanvas()->popClipRect(transform, m_ClipVA);
    }
}

void DivNode::renderOutlines(const VertexArrayPtr& pVA, Pixel32 parentColor)
{
    Pixel32 effColor = getEffectiveOutlineColor(parentColor);
    if (effColor != Pixel32(0,0,0,0)) {
        glm::vec2 size = getSize();
        if (size == glm::vec2(0,0)) {
            glm::vec2 p0 = getAbsPos(glm::vec2(-4, 0.5));
            glm::vec2 p1 = getAbsPos(glm::vec2(5, 0.5));
            glm::vec2 p2 = getAbsPos(glm::vec2(0.5, -4));
            glm::vec2 p3 = getAbsPos(glm::vec2(0.5, 5));
            pVA->addLineData(effColor, p0, p1, 1);
            pVA->addLineData(effColor, p2, p3, 1);
        } else {
            AreaNode::renderOutlines(pVA, parentColor);
        }
    }
    for (unsigned i = 0; i < getNumChildren(); i++) {
        getChild(i)->renderOutlines(pVA, effColor);
    }
}

string DivNode::getEffectiveMediaDir()
{
    // TODO: There is no test for this function.
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
    for(unsigned i = 0; i < getNumChildren(); ++i) {
        getChild(i)->checkReload();
    }
}

string DivNode::dump(int indent)
{
    string dumpStr = AreaNode::dump () + "\n";
    vector<NodePtr>::iterator it;
    for(unsigned i = 0; i < getNumChildren(); ++i) {
        getChild(i)->dump(indent+2)+"\n";
    }
    return dumpStr;
}

IntPoint DivNode::getMediaSize()
{
    return IntPoint(0, 0);
}
 
bool DivNode::isChildTypeAllowed(const string& sType)
{
    return getDefinition()->isChildAllowed(sType);
}

}
