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

using namespace std;
using namespace boost;

#define DEFAULT_SIZE 10000

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

void DivNode::setRenderingEngines(DisplayEngine * pDisplayEngine, 
        AudioEngine * pAudioEngine)
{
    AreaNode::setRenderingEngines(pDisplayEngine, pAudioEngine);
    for (unsigned i = 0; i < getNumChildren(); ++i) {
        getVChild(i)->setRenderingEngines(pDisplayEngine, pAudioEngine);
    }
}

void DivNode::connect(CanvasPtr pCanvas)
{
    AreaNode::connect(pCanvas);
    for (unsigned i = 0; i < getNumChildren(); ++i) {
        getVChild(i)->connect(pCanvas);
    }
}

void DivNode::disconnect(bool bKill)
{
    for (unsigned i = 0; i < getNumChildren(); ++i) {
        getVChild(i)->disconnect(bKill);
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

VisibleNodePtr DivNode::getVChild(unsigned i)
{
    return dynamic_pointer_cast<VisibleNode>(getChild(i));
}

void DivNode::insertChild(NodePtr pNewNode, unsigned i)
{
    VisibleNodePtr pVNode = dynamic_pointer_cast<VisibleNode>(pNewNode);
    if (pVNode->getState() == NS_CONNECTED || pVNode->getState() == NS_CANRENDER)
    {
        throw(Exception(AVG_ERR_ALREADY_CONNECTED,
                "Can't connect node with id "+pNewNode->getID()+
                ": already connected."));
    }
    if (getState() == NS_CONNECTED || getState() == NS_CANRENDER) {
        getCanvas()->registerNode(pVNode);
    }
    DivNodePtr Ptr = dynamic_pointer_cast<DivNode>(getThis());
    pNewNode->checkSetParentError(Ptr); 
    Node::insertChild(pNewNode, i);
    pVNode->setParent(Ptr, getState(), getCanvas());
    if (getState() == NS_CANRENDER) {
        pVNode->setRenderingEngines(getDisplayEngine(), getAudioEngine());
    }
}

void DivNode::removeChild(NodePtr pNode)
{
    removeChild(pNode, false);
}

void DivNode::removeChild(unsigned i)
{
    removeChild(i, false);
}

void DivNode::removeChild(NodePtr pNode, bool bKill)
{
    pNode->Node::setParent(NodePtr());
    VisibleNodePtr pVNode = dynamic_pointer_cast<VisibleNode>(pNode);
    if (pVNode->getState() != NS_UNCONNECTED) {
        pVNode->disconnect(bKill);
    }
    eraseChild(pNode);
}

void DivNode::removeChild(unsigned i, bool bKill)
{
    VisibleNodePtr pNode = getVChild(i);
    removeChild(pNode, bKill);
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

VisibleNodePtr DivNode::getElementByPos(const DPoint & pos)
{
    if (reactsToMouseEvents() &&
            ((getSize() == DPoint(DEFAULT_SIZE, DEFAULT_SIZE) ||
             (pos.x >= 0 && pos.y >= 0 && pos.x < getSize().x && pos.y < getSize().y))))
    {
        for (int i = getNumChildren()-1; i >= 0; i--) {
            VisibleNodePtr pCurChild = getVChild(i);
            DPoint relPos = pCurChild->toLocal(pos);
            VisibleNodePtr pFoundNode = pCurChild->getElementByPos(relPos);
            if (pFoundNode) {
                return pFoundNode;
            }
        }
        // Pos isn't in any of the children.
        if (getSize() == DPoint(DEFAULT_SIZE, DEFAULT_SIZE)) {
            // Explicit width/height not given: div itself doesn't react.
            return VisibleNodePtr();
        } else {
            // Explicit width/height given for div.
            return getVThis();
        }
    } else { 
        return VisibleNodePtr();
    }
}

void DivNode::preRender()
{
    VisibleNode::preRender();
    for (unsigned i = 0; i < getNumChildren(); i++) {
        getVChild(i)->preRender();
    }
}

void DivNode::render(const DRect& rect)
{
    DPoint viewport = getSize();
    if (getCrop()) {
        DRect ClipRect(0, 0, viewport.x, viewport.y);
        getDisplayEngine()->pushClipRect(ClipRect);
    }
    for (unsigned i = 0; i < getNumChildren(); i++) {
        getVChild(i)->maybeRender(rect);
    }
    if (getCrop()) {
        getDisplayEngine()->popClipRect();
    }
}

void DivNode::renderOutlines(VertexArrayPtr pVA, Pixel32 color)
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
        getVChild(i)->renderOutlines(pVA, effColor);
    }
}

string DivNode::getEffectiveMediaDir()
{
    string sMediaDir = m_sMediaDir;
    if (!isAbsPath(sMediaDir)) {
        if (getParent()) {
            sMediaDir = getDivParent()->getEffectiveMediaDir()+m_sMediaDir;
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
        getVChild(i)->checkReload();
    }
}

string DivNode::dump(int indent)
{
    string dumpStr = AreaNode::dump () + "\n";
    vector<VisibleNodePtr>::iterator it;
    for(unsigned i = 0; i < getNumChildren(); ++i) {
        getVChild(i)->dump(indent+2)+"\n";
    }
    return dumpStr;
}

IntPoint DivNode::getMediaSize()
{
    return IntPoint(DEFAULT_SIZE, DEFAULT_SIZE);
}
 
}
