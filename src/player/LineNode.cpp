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

#include "../base/Exception.h"

#include <iostream>
#include <sstream>

using namespace std;

namespace avg {

NodeDefinition LineNode::createDefinition()
{
    return NodeDefinition("line", Node::buildNode<LineNode>)
        .extendDefinition(VectorNode::createDefinition())
        .addArg(Arg<DPoint>("pos1", DPoint(0,0), false, offsetof(LineNode, m_P1)))
        .addArg(Arg<DPoint>("pos2", DPoint(0,0), false, offsetof(LineNode, m_P2)))
        .addArg(Arg<double>("texcoord1", 0, false, offsetof(LineNode, m_TC1)))
        .addArg(Arg<double>("texcoord2", 1, false, offsetof(LineNode, m_TC2)))
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

const DPoint& LineNode::getPos1() const 
{
    return m_P1;
}

void LineNode::setPos1(const DPoint& pt) 
{
    m_P1 = pt;
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

void LineNode::calcVertexes(VertexArrayPtr& pVertexArray, Pixel32 color)
{
    pVertexArray->addLineData(color, m_P1, m_P2, getStrokeWidth(), m_TC1, m_TC2);
}

}
