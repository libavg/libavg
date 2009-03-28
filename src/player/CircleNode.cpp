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

#include "CircleNode.h"

#include "NodeDefinition.h"

#include "../graphics/VertexData.h"
#include "../base/Exception.h"

#include <iostream>
#include <sstream>

using namespace std;

namespace avg {

NodeDefinition CircleNode::createDefinition()
{
    return NodeDefinition("circle", Node::buildNode<CircleNode>)
        .extendDefinition(FilledVectorNode::createDefinition())
        .addArg(Arg<double>("x", 0, false, offsetof(CircleNode, m_Pos.x)))
        .addArg(Arg<double>("y", 0, false, offsetof(CircleNode, m_Pos.y)))
        .addArg(Arg<double>("r", 1, false, offsetof(CircleNode, m_Radius)))
        .addArg(Arg<double>("texcoord1", 0, false, offsetof(CircleNode, m_TC1)))
        .addArg(Arg<double>("texcoord2", 1, false, offsetof(CircleNode, m_TC2)))
        ;
}

CircleNode::CircleNode(const ArgList& Args, bool bFromXML)
    : FilledVectorNode(Args)
{
    Args.setMembers(this);
}

CircleNode::~CircleNode()
{
}

double CircleNode::getX() const 
{
    return m_Pos.x;
}

void CircleNode::setX(double x) 
{
    m_Pos.x = x;
    setDrawNeeded(false);
}

double CircleNode::getY() const 
{
    return m_Pos.y;
}

void CircleNode::setY(double y) 
{
    m_Pos.y = y;
    setDrawNeeded(false);
}

const DPoint& CircleNode::getPos() const 
{
    return m_Pos;
}

void CircleNode::setPos(const DPoint& pt) 
{
    m_Pos = pt;
    setDrawNeeded(false);
}

double CircleNode::getR() const 
{
    return m_Radius;
}

void CircleNode::setR(double r) 
{
    m_Radius = r;
    setDrawNeeded(true);
}

double CircleNode::getTexCoord1() const
{
    return m_TC1;
}

void CircleNode::setTexCoord1(double tc)
{
    m_TC1 = tc;
    setDrawNeeded(false);
}

double CircleNode::getTexCoord2() const
{
    return m_TC2;
}

void CircleNode::setTexCoord2(double tc)
{
    m_TC2 = tc;
    setDrawNeeded(false);
}

NodePtr CircleNode::getElementByPos(const DPoint & pos)
{
    if (calcDist(pos, m_Pos) <= m_Radius)
    {
        return getThis();
    } else {
        return NodePtr();
    }
}

int CircleNode::getNumVertexes()
{
    return (getNumCircumferencePoints()+1)*2;
}

int CircleNode::getNumIndexes()
{
    return getNumCircumferencePoints()*6;
}

int CircleNode::getNumFillVertexes()
{
    return (getNumCircumferencePoints()+1)+1;
}

int CircleNode::getNumFillIndexes()
{
    return getNumCircumferencePoints()*3;
}

void CircleNode::calcVertexes(VertexArrayPtr& pVertexArray, Pixel32 color)
{
    DPoint firstPt1 = getCirclePt(0, m_Radius+getStrokeWidth()/2);
    DPoint firstPt2 = getCirclePt(0, m_Radius-getStrokeWidth()/2);
    int curVertex = 0;
    pVertexArray->appendPos(firstPt1, DPoint(m_TC1, 0), color);
    pVertexArray->appendPos(firstPt2, DPoint(m_TC1, 1), color);
    for (int i=1; i<=getNumCircumferencePoints(); ++i) {
        double ratio = (double(i)/getNumCircumferencePoints());
        double angle = ratio*2*3.14159;
        DPoint curPt1 = getCirclePt(angle, m_Radius+getStrokeWidth()/2);
        DPoint curPt2 = getCirclePt(angle, m_Radius-getStrokeWidth()/2);
        double curTC = (1-ratio)*m_TC1+ratio*m_TC2;
        pVertexArray->appendPos(curPt1, DPoint(curTC, 0), color);
        pVertexArray->appendPos(curPt2, DPoint(curTC, 1), color);
        pVertexArray->appendQuadIndexes(curVertex+1, curVertex, curVertex+3, curVertex+2); 
        curVertex += 2;
    }

}

void CircleNode::calcFillVertexes(VertexArrayPtr& pVertexArray, Pixel32 color)
{
    DPoint minPt = m_Pos-DPoint(m_Radius, m_Radius);
    DPoint maxPt = m_Pos+DPoint(m_Radius, m_Radius);
    DPoint centerTexCoord = calcFillTexCoord(m_Pos, minPt, maxPt);
    pVertexArray->appendPos(m_Pos, centerTexCoord, color);
    int curVertex = 1;
    DPoint firstPt = getCirclePt(0, m_Radius);
    DPoint firstTexCoord = calcFillTexCoord(firstPt, minPt, maxPt);
    pVertexArray->appendPos(firstPt, firstTexCoord, color);
    for (int i=1; i<=getNumCircumferencePoints(); ++i) {
        double ratio = (double(i)/getNumCircumferencePoints());
        double angle = ratio*2*3.14159;
        DPoint curPt = getCirclePt(angle, m_Radius);
        DPoint curTexCoord = calcFillTexCoord(curPt, minPt, maxPt);
        pVertexArray->appendPos(curPt, curTexCoord, color);
        pVertexArray->appendTriIndexes(0, curVertex, curVertex+1);
        curVertex++;
    }
}

int CircleNode::getNumCircumferencePoints()
{
    return int(m_Radius*3);
}

DPoint CircleNode::getCirclePt(double angle, double radius)
{
    return DPoint(sin(angle)*radius, -cos(angle)*radius)+m_Pos;
}

}
