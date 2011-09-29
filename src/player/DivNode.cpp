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
#include "NodeDefinition.h"
#include "Canvas.h"

#include "../base/Point.h"
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

#define DEFAULT_SIZE 100000

namespace avg {


NodeDefinition DivNode::createDefinition()
{
    string sChildArray[] = {"image", "div", "canvas", "words", "video", "camera", 
            "panoimage", "sound", "line", "rect", "curve", "polyline", "polygon",
            "circle", "mesh"};
    vector<string> sChildren = vectorFromCArray(
            sizeof(sChildArray) / sizeof(*sChildArray), sChildArray);
    return NodeDefinition("div", Node::buildNode<DivNode>)
        .extendDefinition(AreaNode::createDefinition())
        .addChildren(sChildren)
        .addArg(Arg<bool>("crop", false, false, offsetof(DivNode, m_bCrop)))
        .addArg(Arg<string>("elementoutlinecolor", "", false, 
                offsetof(DivNode, m_sElementOutlineColor)))
        .addArg(Arg<UTF8String>("mediadir", "", false, offsetof(DivNode, m_sMediaDir)));
}

DivNode::DivNode(const ArgList& args)
{
    args.setMembers(this);
    setElementOutlineColor(m_sElementOutlineColor);
    ObjectCounter::get()->incRef(&typeid(*this));
}

DivNode::~DivNode()
{
    ObjectCounter::get()->decRef(&typeid(*this));
}

void DivNode::connectDisplay()
{
    AreaNode::connectDisplay();
    for (unsigned i = 0; i < getNumChildren(); ++i) {
        getChild(i)->connectDisplay();
    }
    m_pClipVertexes = VertexArrayPtr(new VertexArray());
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
    if (m_pClipVertexes) {
        m_pClipVertexes = VertexArrayPtr();
    }
    AreaNode::disconnect(bKill);
}

DPoint DivNode::getPivot() const
{
    DPoint pivot = AreaNode::getPivot();
    if (pivot == DPoint(DEFAULT_SIZE / 2, DEFAULT_SIZE / 2)) {
        return DPoint(0, 0);
    }
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
    DivNodePtr ptr = dynamic_pointer_cast<DivNode>(shared_from_this());
    pChild->checkSetParentError(ptr); 
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
    pChild->setParent(ptr, getState(), getCanvas());
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

const std::string& DivNode::getElementOutlineColor() const
{
    return m_sElementOutlineColor;
}

void DivNode::setElementOutlineColor(const std::string& sColor)
{
    m_sElementOutlineColor = sColor;
    if (sColor == "") {
        m_ElementOutlineColor = Pixel32(0,0,0,0);
    } else {
        m_ElementOutlineColor = colorStringToColor(m_sElementOutlineColor);
    }
}

const UTF8String& DivNode::getMediaDir() const
{
    return m_sMediaDir;
}

void DivNode::setMediaDir(const UTF8String& sMediaDir)
{
    m_sMediaDir = sMediaDir;
    checkReload();
}

void DivNode::getElementsByPos(const DPoint& pos, vector<NodeWeakPtr>& pElements)
{
    if (reactsToMouseEvents() &&
            ((getSize() == DPoint(DEFAULT_SIZE, DEFAULT_SIZE) ||
             (pos.x >= 0 && pos.y >= 0 && pos.x < getSize().x && pos.y < getSize().y))))
    {
        for (int i = getNumChildren()-1; i >= 0; i--) {
            NodePtr pCurChild = getChild(i);
            DPoint relPos = pCurChild->toLocal(pos);
            pCurChild->getElementsByPos(relPos, pElements);
            if (!pElements.empty()) {
                pElements.push_back(shared_from_this());
                return;
            }
        }
        // pos isn't in any of the children.
        if (getSize() != DPoint(DEFAULT_SIZE, DEFAULT_SIZE)) {
            // Explicit width/height given for div - div reacts on its own.
            pElements.push_back(shared_from_this());
        }
    }
}

void DivNode::preRender()
{
    Node::preRender();
    for (unsigned i = 0; i < getNumChildren(); i++) {
        getChild(i)->preRender();
    }
}

void DivNode::render(const DRect& rect)
{
    DPoint viewport = getSize();
    
    m_pClipVertexes->reset();
    m_pClipVertexes->appendPos(DPoint(0,0), DPoint(0,0), Pixel32(0,0,0,0));
    m_pClipVertexes->appendPos(DPoint(0,viewport.y), DPoint(0,0), Pixel32(0,0,0,0));
    m_pClipVertexes->appendPos(DPoint(viewport.x,0), DPoint(0,0), Pixel32(0,0,0,0));
    m_pClipVertexes->appendPos(viewport, DPoint(0,0), Pixel32(0,0,0,0));
    m_pClipVertexes->appendQuadIndexes(0, 1, 2, 3);

    if (getCrop()) {
        getCanvas()->pushClipRect(m_pClipVertexes);
    }
    for (unsigned i = 0; i < getNumChildren(); i++) {
        getChild(i)->maybeRender(rect);
    }
    if (getCrop()) {
        getCanvas()->popClipRect(m_pClipVertexes);
    }
}

void DivNode::renderOutlines(const VertexArrayPtr& pVA, Pixel32 color)
{
    Pixel32 effColor = color;
    if (m_ElementOutlineColor != Pixel32(0,0,0,0)) {
        effColor = m_ElementOutlineColor;
        effColor.setA(128);
    }
    if (effColor != Pixel32(0,0,0,0)) {
        DPoint size = getSize();
        if (size == DPoint(DEFAULT_SIZE, DEFAULT_SIZE)) {
            DPoint p0 = getAbsPos(DPoint(-4, 0.5));
            DPoint p1 = getAbsPos(DPoint(5, 0.5));
            DPoint p2 = getAbsPos(DPoint(0.5, -4));
            DPoint p3 = getAbsPos(DPoint(0.5, 5));
            pVA->addLineData(effColor, p0, p1, 1);
            pVA->addLineData(effColor, p2, p3, 1);
        } else {
            DPoint p0 = getAbsPos(DPoint(0.5, 0.5));
            DPoint p1 = getAbsPos(DPoint(size.x+0.5,0.5));
            DPoint p2 = getAbsPos(DPoint(size.x+0.5,size.y+0.5));
            DPoint p3 = getAbsPos(DPoint(0.5,size.y+0.5));
            pVA->addLineData(effColor, p0, p1, 1);
            pVA->addLineData(effColor, p1, p2, 1);
            pVA->addLineData(effColor, p2, p3, 1);
            pVA->addLineData(effColor, p3, p0, 1);
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
    return IntPoint(DEFAULT_SIZE, DEFAULT_SIZE);
}
 
bool DivNode::isChildTypeAllowed(const string& sType)
{
    return getDefinition()->isChildAllowed(sType);
}

}
