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

#include "AreaNode.h"

#include "Event.h"
#include "CursorEvent.h"
#include "MouseEvent.h"
#include "DivNode.h"
#include "ArgList.h"
#include "TypeDefinition.h"
#include "TypeRegistry.h"
#include "BoostPython.h"

#include "../base/MathHelper.h"
#include "../base/Logger.h"
#include "../base/Exception.h"
#include "../base/ObjectCounter.h"

#include "../graphics/GLContext.h"

#include <object.h>
#include <compile.h>
#include <eval.h>

#include <iostream>

using namespace boost;

using namespace std;

namespace avg {

void AreaNode::registerType()
{
    TypeDefinition def = TypeDefinition("areanode", "node")
        .addArg(Arg<float>("x", 0.0, false, offsetof(AreaNode, m_RelViewport.tl.x)))
        .addArg(Arg<float>("y", 0.0, false, offsetof(AreaNode, m_RelViewport.tl.y)))
        .addArg(Arg<glm::vec2>("pos", glm::vec2(0.0, 0.0)))
        .addArg(Arg<float>("width", 0.0, false, offsetof(AreaNode, m_UserSize.x)))
        .addArg(Arg<float>("height", 0.0, false, offsetof(AreaNode, m_UserSize.y)))
        .addArg(Arg<glm::vec2>("size", glm::vec2(0.0, 0.0)))
        .addArg(Arg<float>("angle", 0.0, false, offsetof(AreaNode, m_Angle)))
        .addArg(Arg<glm::vec2>("pivot", glm::vec2(-32767, -32767), false, 
                offsetof(AreaNode, m_Pivot)))
        .addArg(Arg<string>("elementoutlinecolor", "", false, 
                offsetof(AreaNode, m_sElementOutlineColor)));
    TypeRegistry::get()->registerType(def);
}

AreaNode::AreaNode()
    : m_RelViewport(0,0,0,0),
      m_Transform(glm::mat4(0)),
      m_bTransformChanged(true)
{
    ObjectCounter::get()->incRef(&typeid(*this));
}

AreaNode::~AreaNode()
{
    ObjectCounter::get()->decRef(&typeid(*this));
}

void AreaNode::setArgs(const ArgList& args)
{
    Node::setArgs(args);
    args.getOverlayedArgVal(&m_RelViewport.tl, "pos", "x", "y", getID());
    args.getOverlayedArgVal(&m_UserSize, "size", "width", "height", getID());
    m_RelViewport.setWidth(m_UserSize.x);
    m_RelViewport.setHeight(m_UserSize.y);
    m_bHasCustomPivot = ((m_Pivot.x != -32767) && (m_Pivot.y != -32767));
    setElementOutlineColor(m_sElementOutlineColor);
}

void AreaNode::connectDisplay()
{
    IntPoint MediaSize = getMediaSize();
    if (m_UserSize.x == 0.0) {
        m_RelViewport.setWidth(float(MediaSize.x));
    } else {
        m_RelViewport.setWidth(float(m_UserSize.x));
    }
    if (m_UserSize.y == 0.0) {
        m_RelViewport.setHeight(float(MediaSize.y));
    } else {
        m_RelViewport.setHeight(float(m_UserSize.y));
    }
    if (m_UserSize.x == 0.0 || m_UserSize.y == 0) {
        notifySubscribers("SIZE_CHANGED", m_RelViewport.size());
    }
    m_bTransformChanged = true;
    Node::connectDisplay();
}

float AreaNode::getX() const 
{
    return m_RelViewport.tl.x;
}

void AreaNode::setX(float x) 
{
    setViewport(x, -32767, -32767, -32767);
}

float AreaNode::getY() const 
{
    return m_RelViewport.tl.y;
}

void AreaNode::setY(float y) 
{
    setViewport(-32767, y, -32767, -32767);
}

const glm::vec2& AreaNode::getPos() const
{
    return m_RelViewport.tl;
}

void AreaNode::setPos(const glm::vec2& pt)
{
    setViewport(pt.x, pt.y, -32767, -32767);
}

float AreaNode::getWidth() const 
{
    return getRelViewport().width();
}

void AreaNode::setWidth(float width) 
{
    m_UserSize.x = width;
    setViewport(-32767, -32767, -32767, -32767);
}

float AreaNode::getHeight() const
{
    return getRelViewport().height();
}

void AreaNode::setHeight(float height) 
{
    m_UserSize.y = height;
    setViewport(-32767, -32767, -32767, -32767);
}

glm::vec2 AreaNode::getSize() const
{
    return getRelViewport().size();
}

void AreaNode::setSize(const glm::vec2& pt)
{
    m_UserSize = pt;
    setViewport(-32767, -32767, -32767, -32767);
}

float AreaNode::getAngle() const
{
    return m_Angle;
}

void AreaNode::setAngle(float angle)
{
    m_Angle = fmod(angle, 2*PI);
    m_bTransformChanged = true;
}

glm::vec2 AreaNode::getPivot() const
{
    if (m_bHasCustomPivot) {
        return m_Pivot;
    } else {
        return getSize()/2.f;
    }
}

void AreaNode::setPivot(const glm::vec2& pt)
{
    m_Pivot.x = pt.x;
    m_Pivot.y = pt.y;
    m_bHasCustomPivot = true;
    m_bTransformChanged = true;
}

const std::string& AreaNode::getElementOutlineColor() const
{
    return m_sElementOutlineColor;
}

void AreaNode::setElementOutlineColor(const std::string& sColor)
{
    m_sElementOutlineColor = sColor;
    if (sColor == "") {
        m_ElementOutlineColor = Pixel32(0,0,0,0);
    } else {
        m_ElementOutlineColor = colorStringToColor(m_sElementOutlineColor);
    }
}

glm::vec2 AreaNode::toLocal(const glm::vec2& globalPos) const
{
    glm::vec2 localPos = globalPos-m_RelViewport.tl;
    return getRotatedPivot(localPos, -getAngle(), getPivot());
}

glm::vec2 AreaNode::toGlobal(const glm::vec2& localPos) const
{
    glm::vec2 globalPos = getRotatedPivot(localPos, getAngle(), getPivot());
    return globalPos+m_RelViewport.tl;
}

void AreaNode::getElementsByPos(const glm::vec2& pos, vector<NodePtr>& pElements)
{
    if (pos.x >= 0 && pos.y >= 0 && pos.x < getSize().x && pos.y < getSize().y &&
            reactsToMouseEvents())
    {
        pElements.push_back(getSharedThis());
    }
}

void AreaNode::maybeRender(const glm::mat4& parentTransform)
{
    AVG_ASSERT(getState() == NS_CANRENDER);
    if (isVisible()) {
        calcTransform();
        m_Transform = parentTransform*m_LocalTransform;
        render();
    }
}

void AreaNode::renderOutlines(const VertexArrayPtr& pVA, Pixel32 parentColor)
{
    Pixel32 effColor = getEffectiveOutlineColor(parentColor);
    if (effColor != Pixel32(0,0,0,0)) {
        glm::vec2 size = getSize();
        glm::vec2 p0 = getAbsPos(glm::vec2(0.5, 0.5));
        glm::vec2 p1 = getAbsPos(glm::vec2(size.x+0.5,0.5));
        glm::vec2 p2 = getAbsPos(glm::vec2(size.x+0.5,size.y+0.5));
        glm::vec2 p3 = getAbsPos(glm::vec2(0.5,size.y+0.5));
        pVA->addLineData(effColor, p0, p1, 1);
        pVA->addLineData(effColor, p1, p2, 1);
        pVA->addLineData(effColor, p2, p3, 1);
        pVA->addLineData(effColor, p3, p0, 1);
    }
}

void AreaNode::setViewport(float x, float y, float width, float height)
{
    glm::vec2 oldSize = getRelViewport().size();
    if (x == -32767) {
        x = getRelViewport().tl.x;
    }
    if (y == -32767) {
        y = getRelViewport().tl.y;
    }
    glm::vec2 mediaSize = glm::vec2(getMediaSize());
    if (width == -32767) {
        if (m_UserSize.x == 0.0) {
            width = mediaSize.x;
        } else {
            width = m_UserSize.x;
        } 
    }
    if (height == -32767) {
        if (m_UserSize.y == 0.0) {
            height = mediaSize.y;
        } else {
            height = m_UserSize.y;
        } 
    }
    if (width < 0 || height < 0) {
        throw Exception(AVG_ERR_OUT_OF_RANGE, "Negative size for a node.");
    }
    m_RelViewport = FRect(x, y, x+width, y+height);
    if (oldSize != m_RelViewport.size()) {
        notifySubscribers("SIZE_CHANGED", m_RelViewport.size());
    }
    m_bTransformChanged = true;
}

const FRect& AreaNode::getRelViewport() const
{
//    cerr << "Node " << getID() << ": " << m_RelViewport << endl;
    return m_RelViewport;
}

string AreaNode::dump(int indent)
{
    string dumpStr = Node::dump(indent); 
    char sz[256];
    sprintf (sz, ", x=%.1f, y=%.1f, width=%.1f, height=%.1f\n",
            m_RelViewport.tl.x, m_RelViewport.tl.y,
            m_RelViewport.width(), m_RelViewport.height());
    dumpStr += sz;

    return dumpStr; 
}

const glm::mat4& AreaNode::getTransform() const
{
    return m_Transform;
}

glm::vec2 AreaNode::getUserSize() const
{
    return m_UserSize;
}

Pixel32 AreaNode::getEffectiveOutlineColor(Pixel32 parentColor) const
{
    if (m_ElementOutlineColor == Pixel32(0,0,0,0)) {
        return parentColor;
    } else {
        return m_ElementOutlineColor;
    }
}

void AreaNode::calcTransform()
{
    if (m_bTransformChanged) {
        glm::vec3 pos(m_RelViewport.tl.x, m_RelViewport.tl.y, 0);
        glm::vec3 pivot(getPivot().x, getPivot().y, 0);
        glm::mat4 transform = glm::translate(glm::mat4(1.0f), pos);
        transform = glm::translate(transform, pivot);
        transform = glm::rotate(transform, m_Angle, glm::vec3(0,0,1));
        m_LocalTransform = glm::translate(transform, -pivot);
        m_bTransformChanged = false;
    }
}

}
