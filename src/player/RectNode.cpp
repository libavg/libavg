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

#include "RectNode.h"

#include "NodeDefinition.h"

#include "../graphics/VertexArray.h"
#include "../base/Exception.h"
#include "../base/MathHelper.h"

#include <iostream>
#include <sstream>

using namespace std;

namespace avg {

NodeDefinition RectNode::createDefinition()
{
    return NodeDefinition("rect", Node::buildNode<RectNode>)
        .extendDefinition(VectorNode::createDefinition())
        .addArg(Arg<double>("x", 0, false, offsetof(RectNode, m_Rect.tl.x)))
        .addArg(Arg<double>("y", 0, false, offsetof(RectNode, m_Rect.tl.y)))
        .addArg(Arg<double>("width", 0))
        .addArg(Arg<double>("height", 0))
        .addArg(Arg<double>("angle", 0.0, false, offsetof(RectNode, m_Angle)))
        .addArg(Arg<double>("fillopacity", 0, false, 
                offsetof(RectNode, m_FillOpacity)))
        .addArg(Arg<string>("fillcolor", "FFFFFF", false, 
                offsetof(RectNode, m_sFillColorName)));
}

RectNode::RectNode(const ArgList& Args, bool bFromXML)
    : VectorNode(Args)
{
    Args.setMembers(this);
    m_Rect.setWidth(Args.getArgVal<double>("width"));
    m_Rect.setHeight(Args.getArgVal<double>("height"));
    m_FillColor = colorStringToColor(m_sFillColorName);
}

RectNode::~RectNode()
{
}

double RectNode::getX() const 
{
    return m_Rect.tl.x;
}

void RectNode::setX(double x) 
{
    double w = m_Rect.width();
    m_Rect.tl.x = x;
    m_Rect.setWidth(w);
    setDrawNeeded(true);
}

double RectNode::getY() const 
{
    return m_Rect.tl.y;
}

void RectNode::setY(double y) 
{
    double h = m_Rect.height();
    m_Rect.tl.y = y;
    m_Rect.setHeight(h);
    setDrawNeeded(true);
}

const DPoint& RectNode::getPos() const 
{
    return m_Rect.tl;
}

void RectNode::setPos(const DPoint& pt) 
{
    double w = m_Rect.width();
    double h = m_Rect.height();
    m_Rect.tl = pt;
    m_Rect.setHeight(w);
    m_Rect.setHeight(h);
    setDrawNeeded(true);
}

double RectNode::getWidth() const
{
    return m_Rect.width();
}

void RectNode::setWidth(double w)
{
    m_Rect.setWidth(w);
    setDrawNeeded(true);
}

double RectNode::getHeight() const
{
    return m_Rect.height();
}

void RectNode::setHeight(double h)
{
    m_Rect.setHeight(h);
    setDrawNeeded(true);
}

DPoint RectNode::getSize() const 
{
    return m_Rect.size();
}

void RectNode::setSize(const DPoint& pt) 
{
    m_Rect.setWidth(pt.x);
    m_Rect.setHeight(pt.y);
    setDrawNeeded(true);
}

double RectNode::getAngle() const
{
    return m_Angle;
}

void RectNode::setAngle(double angle)
{
    m_Angle = fmod(angle, 2*PI);
    setDrawNeeded(true);
}

double RectNode::getFillOpacity() const
{
    return m_FillOpacity;
}

void RectNode::setFillOpacity(double opacity)
{
    m_FillOpacity = opacity;
    setDrawNeeded(true);
}

void RectNode::setFillColor(const string& sFillColor)
{
    if (m_sFillColorName != sFillColor) {
        m_sFillColorName = sFillColor;
        m_FillColor = colorStringToColor(m_sFillColorName);
        setDrawNeeded(true);
    }
}

const string& RectNode::getFillColor() const
{
    return m_sFillColorName;
}

int RectNode::getNumVertexes()
{
    return 4*5;
}

int RectNode::getNumIndexes()
{
    return 6*5;
}

void RectNode::updateData(VertexArrayPtr pVertexArray, int curVertex, int curIndex, 
        double opacity, bool bParentDrawNeeded)
{
    if (isDrawNeeded() || bParentDrawNeeded) {
        double curOpacity = opacity*m_FillOpacity;
        Pixel32 color = m_FillColor;
        color.setA((unsigned char)(curOpacity*255));

        DPoint pivot = m_Rect.tl+m_Rect.size()/2;

        DPoint p1 = m_Rect.tl;
        DPoint p2(m_Rect.tl.x, m_Rect.br.y);
        DPoint p3 = m_Rect.br;
        DPoint p4(m_Rect.br.x, m_Rect.tl.y);
        DPoint rp1 = rotate(p1, m_Angle, pivot); 
        DPoint rp2 = rotate(p2, m_Angle, pivot); 
        DPoint rp3 = rotate(p3, m_Angle, pivot); 
        DPoint rp4 = rotate(p4, m_Angle, pivot); 
        pVertexArray->setPos(curVertex, rp1, DPoint(0,0), color);
        pVertexArray->setPos(curVertex+1, rp2, DPoint(0,0), color);
        pVertexArray->setPos(curVertex+2, rp3, DPoint(0,0), color);
        pVertexArray->setPos(curVertex+3, rp4, DPoint(0,0), color);
        pVertexArray->setIndex(curIndex, curVertex);
        pVertexArray->setIndex(curIndex+1, curVertex+1);
        pVertexArray->setIndex(curIndex+2, curVertex+2);
        pVertexArray->setIndex(curIndex+3, curVertex);
        pVertexArray->setIndex(curIndex+4, curVertex+2);
        pVertexArray->setIndex(curIndex+5, curVertex+3);

        updateLineData(pVertexArray, curVertex+4, curIndex+6, opacity, rp1, rp2);
        updateLineData(pVertexArray, curVertex+8, curIndex+12, opacity, rp3, rp4);
        p1.x -= getStrokeWidth()/2;
        p2.x -= getStrokeWidth()/2;
        p3.x += getStrokeWidth()/2;
        p4.x += getStrokeWidth()/2;
        rp1 = rotate(p1, m_Angle, pivot); 
        rp2 = rotate(p2, m_Angle, pivot); 
        rp3 = rotate(p3, m_Angle, pivot); 
        rp4 = rotate(p4, m_Angle, pivot); 
        updateLineData(pVertexArray, curVertex+12, curIndex+18, opacity, rp2, rp3);
        updateLineData(pVertexArray, curVertex+16, curIndex+24, opacity, rp4, rp1);
    }
    setDrawNeeded(false);
}

}
