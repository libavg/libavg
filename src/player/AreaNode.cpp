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

#include "AreaNode.h"

#include "Event.h"
#include "CursorEvent.h"
#include "MouseEvent.h"
#include "DivNode.h"
#include "ArgList.h"
#include "NodeDefinition.h"
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
using namespace boost::python;

using namespace std;

namespace avg {

NodeDefinition AreaNode::createDefinition()
{
    return NodeDefinition("areanode")
        .extendDefinition(Node::createDefinition())
        .addArg(Arg<float>("x", 0.0, false, offsetof(AreaNode, m_RelViewport.tl.x)))
        .addArg(Arg<float>("y", 0.0, false, offsetof(AreaNode, m_RelViewport.tl.y)))
        .addArg(Arg<glm::vec2>("pos", glm::vec2(0.0, 0.0)))
        .addArg(Arg<float>("width", 0.0, false, offsetof(AreaNode, m_UserSize.x)))
        .addArg(Arg<float>("height", 0.0, false, offsetof(AreaNode, m_UserSize.y)))
        .addArg(Arg<glm::vec2>("size", glm::vec2(0.0, 0.0)))
        .addArg(Arg<float>("angle", 0.0, false, offsetof(AreaNode, m_Angle)))
        .addArg(Arg<glm::vec2>("pivot", glm::vec2(-32767, -32767), false, 
                offsetof(AreaNode, m_Pivot)));
}

AreaNode::AreaNode()
    : m_RelViewport(0,0,0,0),
      m_Transform(glm::mat4(0))
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

void AreaNode::getElementsByPos(const glm::vec2& pos, vector<NodeWeakPtr>& pElements)
{
    if (pos.x >= 0 && pos.y >= 0 && pos.x < getSize().x && pos.y < getSize().y &&
            reactsToMouseEvents())
    {
        pElements.push_back(shared_from_this());
    }
}

void AreaNode::maybeRender()
{
    AVG_ASSERT(getState() == NS_CANRENDER);
    if (isVisible()) {
        if (getID() != "") {
            AVG_TRACE(Logger::BLTS, "Rendering " << getTypeStr() << 
                    " with ID " << getID());
        } else {
            AVG_TRACE(Logger::BLTS, "Rendering " << getTypeStr()); 
        }
        m_Transform = getParentTransform()*calcTransform();
        glLoadMatrixf(glm::value_ptr(m_Transform));
        render();
    }
}

void AreaNode::setViewport(float x, float y, float width, float height)
{
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

glm::mat4 AreaNode::calcTransform()
{
    glm::vec3 pos(m_RelViewport.tl.x, m_RelViewport.tl.y, 0);
    glm::vec3 pivot(getPivot().x, getPivot().y, 0);
    glm::mat4 transform = glm::translate(glm::mat4(1.0f), pos);
    transform = glm::translate(transform, pivot);
    transform = glm::rotate(transform, (180.f/PI)*m_Angle, glm::vec3(0,0,1));
    transform = glm::translate(transform, -pivot);
    return transform;
}

}
