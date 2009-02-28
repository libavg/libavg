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

#include "CurveNode.h"

#include "NodeDefinition.h"

#include "../graphics/VertexArray.h"
#include "../base/Exception.h"
#include "../base/MathHelper.h"
#include "../base/BezierCurve.h"

#include <math.h>
#include <float.h>
#include <iostream>
#include <sstream>

using namespace std;

namespace avg {

NodeDefinition CurveNode::createDefinition()
{
    return NodeDefinition("curve", Node::buildNode<CurveNode>)
        .extendDefinition(VectorNode::createDefinition())
        .addArg(Arg<double>("x1", 0, true, offsetof(CurveNode, m_P1.x)))
        .addArg(Arg<double>("y1", 0, true, offsetof(CurveNode, m_P1.y)))
        .addArg(Arg<double>("x2", 0, true, offsetof(CurveNode, m_P2.x)))
        .addArg(Arg<double>("y2", 0, true, offsetof(CurveNode, m_P2.y)))
        .addArg(Arg<double>("x3", 0, true, offsetof(CurveNode, m_P3.x)))
        .addArg(Arg<double>("y3", 0, true, offsetof(CurveNode, m_P3.y)))
        .addArg(Arg<double>("x4", 0, true, offsetof(CurveNode, m_P4.x)))
        .addArg(Arg<double>("y4", 0, true, offsetof(CurveNode, m_P4.y)))
        .addArg(Arg<double>("texcoord1", 0, true, offsetof(CurveNode, m_TC1)))
        .addArg(Arg<double>("texcoord2", 1, true, offsetof(CurveNode, m_TC2)));
}

CurveNode::CurveNode(const ArgList& Args, bool bFromXML)
   : VectorNode(Args)
{
    Args.setMembers(this);
}

CurveNode::~CurveNode()
{
}

double CurveNode::getX1() const 
{
    return m_P1.x;
}

void CurveNode::setX1(double x) 
{
    m_P1.x = x;
    setDrawNeeded(true);
}

double CurveNode::getY1() const 
{
    return m_P1.y;
}

void CurveNode::setY1(double y) 
{
    m_P1.y = y;
    setDrawNeeded(true);
}

const DPoint& CurveNode::getPos1() const 
{
    return m_P1;
}

void CurveNode::setPos1(const DPoint& pt) 
{
    m_P1 = pt;
    setDrawNeeded(true);
}

double CurveNode::getX2() const 
{
    return m_P2.x;
}

void CurveNode::setX2(double x) 
{
    m_P2.x = x;
    setDrawNeeded(true);
}

double CurveNode::getY2() const 
{
    return m_P2.y;
}

void CurveNode::setY2(double y) 
{
    m_P2.y = y;
    setDrawNeeded(true);
}

const DPoint& CurveNode::getPos2() const 
{
    return m_P2;
}

void CurveNode::setPos2(const DPoint& pt) 
{
    m_P2 = pt;
    setDrawNeeded(true);
}

double CurveNode::getX3() const 
{
    return m_P3.x;
}

void CurveNode::setX3(double x) 
{
    m_P3.x = x;
    setDrawNeeded(true);
}

double CurveNode::getY3() const 
{
    return m_P3.y;
}

void CurveNode::setY3(double y) 
{
    m_P3.y = y;
    setDrawNeeded(true);
}

const DPoint& CurveNode::getPos3() const 
{
    return m_P3;
}

void CurveNode::setPos3(const DPoint& pt) 
{
    m_P3 = pt;
    setDrawNeeded(true);
}

double CurveNode::getX4() const 
{
    return m_P4.x;
}

void CurveNode::setX4(double x) 
{
    m_P4.x = x;
    setDrawNeeded(true);
}

double CurveNode::getY4() const 
{
    return m_P4.y;
}

void CurveNode::setY4(double y) 
{
    m_P4.y = y;
    setDrawNeeded(true);
}

const DPoint& CurveNode::getPos4() const 
{
    return m_P4;
}

void CurveNode::setPos4(const DPoint& pt) 
{
    m_P4 = pt;
    setDrawNeeded(true);
}

double CurveNode::getTexCoord1() const
{
    return m_TC1;
}

void CurveNode::setTexCoord1(double tc)
{
    m_TC1 = tc;
    setDrawNeeded(false);
}

double CurveNode::getTexCoord2() const
{
    return m_TC2;
}

void CurveNode::setTexCoord2(double tc)
{
    m_TC2 = tc;
    setDrawNeeded(false);
}

int CurveNode::getNumVertexes()
{
    return (getCurveLen()+1)*2;
}

int CurveNode::getNumIndexes()
{
    return (getCurveLen())*2*3;
}

void CurveNode::calcVertexes(VertexArrayPtr& pVertexArray, double opacity)
{
    updateLines();
    double curOpacity = opacity*getOpacity();
    Pixel32 color = getColorVal();
    color.setA((unsigned char)(curOpacity*255));

    pVertexArray->appendPos(m_LeftCurve[0], DPoint(m_TC1,1), color);
    pVertexArray->appendPos(m_RightCurve[0], DPoint(m_TC2,0), color);
    for (unsigned i=0; i<m_LeftCurve.size()-1; ++i) {
        double ratio = i/double(m_LeftCurve.size());
        double tc = (1-ratio)*m_TC1+ratio*m_TC2;
        pVertexArray->appendPos(m_LeftCurve[i+1], DPoint(tc,1), color);
        pVertexArray->appendPos(m_RightCurve[i+1], DPoint(tc,0), color);
        pVertexArray->appendQuadIndexes((i+1)*2, i*2, (i+1)*2+1, i*2+1);
    }
}

int CurveNode::getCurveLen()
{
    // Calc. upper bound for spline length.
    double curveLen = calcDist(m_P2,m_P1)+calcDist(m_P3,m_P2)+calcDist(m_P4,m_P3);
    if (curveLen > 50000) {
        throw Exception(AVG_ERR_OUT_OF_RANGE, "Illegal points in curve.");
    }
    return int(curveLen);
}

void CurveNode::updateLines()
{
    BezierCurve curve(m_P1, m_P2, m_P3, m_P4);
    
    double len = getCurveLen();
    m_LeftCurve.clear();
    m_RightCurve.clear();
    m_LeftCurve.reserve(int(len+1.5));
    m_RightCurve.reserve(int(len+1.5));

    for (unsigned i=0; i<len; ++i) {
        double t = i/len;
        addLRCurvePoint(curve.interpolate(t), curve.getDeriv(t));
    }
    addLRCurvePoint(curve.interpolate(1), curve.getDeriv(1));
}

void CurveNode::addLRCurvePoint(const DPoint& pos, const DPoint& deriv)
{
    DPoint m(deriv);
    m.normalize();
    DPoint w = DPoint(m.y, -m.x)*getStrokeWidth()/2;
    m_LeftCurve.push_back(pos-w);
    m_RightCurve.push_back(pos+w);
}

}
