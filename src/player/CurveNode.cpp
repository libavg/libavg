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
#include "../base/CubicSpline.h"

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
        .addArg(Arg<double>("y4", 0, true, offsetof(CurveNode, m_P4.y)));
}

CurveNode::CurveNode(const ArgList& Args, bool bFromXML)
    : VectorNode(Args)
{
    Args.setMembers(this);
}

CurveNode::~CurveNode()
{
}

void CurveNode::setRenderingEngines(DisplayEngine * pDisplayEngine, 
        AudioEngine * pAudioEngine)
{
    setDrawNeeded(true);
    VectorNode::setRenderingEngines(pDisplayEngine, pAudioEngine);
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

int CurveNode::getNumTriangles()
{
    return (getCurveLen())*2;
}

void CurveNode::updateData(VertexArrayPtr pVertexArray, int triIndex, double opacity, 
        bool bParentDrawNeeded)
{
    if (isDrawNeeded() || bParentDrawNeeded) {
        updateLines();
        double curOpacity = opacity*getOpacity();
        Pixel32 color = getColorVal();
        color.setA(unsigned char(curOpacity*255));
        for (unsigned i=0; i<m_LeftCurve.size()-1; ++i) {
            const DPoint& p1 = m_LeftCurve[i];
            const DPoint& p2 = m_LeftCurve[i+1];
            const DPoint& p3 = m_RightCurve[i+1];
            const DPoint& p4 = m_RightCurve[i];
            pVertexArray->setPos(triIndex+i*2, 0, p1, DPoint(0,0), color);
            pVertexArray->setPos(triIndex+i*2, 1, p2, DPoint(0,0), color);
            pVertexArray->setPos(triIndex+i*2, 2, p3, DPoint(0,0), color);

            pVertexArray->setPos(triIndex+i*2+1, 0, p1, DPoint(0,0), color);
            pVertexArray->setPos(triIndex+i*2+1, 1, p3, DPoint(0,0), color);
            pVertexArray->setPos(triIndex+i*2+1, 2, p4, DPoint(0,0), color);
        }
    }
    setDrawNeeded(false);
}

int CurveNode::getCurveLen()
{
    // Calc. upper bound for spline length.
    return int(calcDist(m_P2,m_P1)+calcDist(m_P3,m_P2)+calcDist(m_P4,m_P3));
}

void CurveNode::updateLines()
{
    // Generate control points the way CubicSpline.cpp wants.
    static double ControlPoints[] = {-1, 0, 1, 2};
    double xPoints[] = {2*m_P1.x-m_P2.x, m_P1.x, m_P4.x, 2*m_P4.x-m_P3.x};
    double yPoints[] = {2*m_P1.y-m_P2.y, m_P1.y, m_P4.y, 2*m_P4.y-m_P3.y};
    vector<double> splineControl = vectorFromCArray(4, ControlPoints);
    vector<double> splineX = vectorFromCArray(4, xPoints);
    CubicSpline xSpline(splineControl, splineX);
    vector<double> splineY = vectorFromCArray(4, yPoints);
    CubicSpline ySpline(splineControl, splineY);
    
    // Calc. upper bound for spline length.
    double len = getCurveLen();
    
    vector<DPoint> centerCurve;
    for (int i=0; i<len; ++i) {
        DPoint curPt(xSpline.interpolate(i/len), ySpline.interpolate(i/len));
        centerCurve.push_back(curPt);
    }
    centerCurve.push_back(m_P4);

    m_LeftCurve.clear();
    m_RightCurve.clear();

    addLRCurvePoint(centerCurve[0], centerCurve[1]-centerCurve[0]);
    for (unsigned i=1; i<centerCurve.size()-1; ++i) {
        addLRCurvePoint(centerCurve[i], centerCurve[i-1]-centerCurve[i+1]);
    }
    unsigned l = centerCurve.size();
    addLRCurvePoint(centerCurve[l-1], centerCurve[l-2]-centerCurve[l-1]);
}

void CurveNode::addLRCurvePoint(const DPoint& pos, const DPoint& delta)
{
    // TODO: Use correct derivative of the curve.
    DPoint m(delta);
    m.normalize();
    DPoint w = DPoint(m.y, -m.x)*getStrokeWidth()/2;
    m_LeftCurve.push_back(pos-w);
    m_RightCurve.push_back(pos+w);
}

}
