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

#include "LineNode.h"

#include "NodeDefinition.h"

#include "../graphics/VertexData.h"
#include "../base/Exception.h"

#include <iostream>
#include <sstream>

using namespace std;

namespace avg {

NodeDefinition LineNode::createDefinition()
{
    return NodeDefinition("line", Node::buildNode<LineNode>)
        .extendDefinition(VectorNode::createDefinition())
        .addArg(Arg<double>("x1", 0, true, offsetof(LineNode, m_P1.x)))
        .addArg(Arg<double>("y1", 0, true, offsetof(LineNode, m_P1.y)))
        .addArg(Arg<double>("x2", 0, true, offsetof(LineNode, m_P2.x)))
        .addArg(Arg<double>("y2", 0, true, offsetof(LineNode, m_P2.y)))
        .addArg(Arg<double>("texcoord1", 0, true, offsetof(LineNode, m_TC1)))
        .addArg(Arg<double>("texcoord2", 1, true, offsetof(LineNode, m_TC2)))
        ;
}

LineNode::LineNode(const ArgList& Args, bool bFromXML)
    : VectorNode(Args)
{
    Args.setMembers(this);
}

LineNode::~LineNode()
{
}

double LineNode::getX1() const 
{
    return m_P1.x;
}

void LineNode::setX1(double x) 
{
    m_P1.x = x;
    setDrawNeeded(false);
}

double LineNode::getY1() const 
{
    return m_P1.y;
}

void LineNode::setY1(double y) 
{
    m_P1.y = y;
    setDrawNeeded(false);
}

const DPoint& LineNode::getPos1() const 
{
    return m_P1;
}

void LineNode::setPos1(const DPoint& pt) 
{
    m_P1 = pt;
    setDrawNeeded(false);
}

double LineNode::getX2() const 
{
    return m_P2.x;
}

void LineNode::setX2(double x) 
{
    m_P2.x = x;
    setDrawNeeded(false);
}

double LineNode::getY2() const 
{
    return m_P2.y;
}

void LineNode::setY2(double y) 
{
    m_P2.y = y;
    setDrawNeeded(false);
}

const DPoint& LineNode::getPos2() const 
{
    return m_P2;
}

void LineNode::setPos2(const DPoint& pt) 
{
    m_P2 = pt;
    setDrawNeeded(false);
}

double LineNode::getTexCoord1() const
{
    return m_TC1;
}

void LineNode::setTexCoord1(double tc)
{
    m_TC1 = tc;
    setDrawNeeded(false);
}

double LineNode::getTexCoord2() const
{
    return m_TC2;
}

void LineNode::setTexCoord2(double tc)
{
    m_TC2 = tc;
    setDrawNeeded(false);
}

int LineNode::getNumVertexes()
{
    return 4;
}

int LineNode::getNumIndexes()
{
    return 6;
}

void LineNode::calcVertexes(VertexArrayPtr& pVertexArray, 
                VertexArrayPtr& pFillVertexArray, double opacity)
{
    updateLineData(pVertexArray, opacity, m_P1, m_P2, m_TC1, m_TC2);
}

}
