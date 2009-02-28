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
        .extendDefinition(FilledVectorNode::createDefinition())
        .addArg(Arg<double>("x", 0, false, offsetof(RectNode, m_Rect.tl.x)))
        .addArg(Arg<double>("y", 0, false, offsetof(RectNode, m_Rect.tl.y)))
        .addArg(Arg<double>("width", 0))
        .addArg(Arg<double>("height", 0))
        .addArg(Arg<double>("angle", 0.0, false, offsetof(RectNode, m_Angle)))
        ;
}

RectNode::RectNode(const ArgList& Args, bool bFromXML)
    : FilledVectorNode(Args)
{
    Args.setMembers(this);
    m_Rect.setWidth(Args.getArgVal<double>("width"));
    m_Rect.setHeight(Args.getArgVal<double>("height"));
    double texCoords[] = {0, 0.25, 0.5, 0.75, 1};
    m_TexCoords = vectorFromCArray(5, texCoords);
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
    setDrawNeeded(false);
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
    setDrawNeeded(false);
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
    m_Rect.setWidth(w);
    m_Rect.setHeight(h);
    setDrawNeeded(false);
}

double RectNode::getWidth() const
{
    return m_Rect.width();
}

void RectNode::setWidth(double w)
{
    m_Rect.setWidth(w);
    setDrawNeeded(false);
}

double RectNode::getHeight() const
{
    return m_Rect.height();
}

void RectNode::setHeight(double h)
{
    m_Rect.setHeight(h);
    setDrawNeeded(false);
}

DPoint RectNode::getSize() const 
{
    return m_Rect.size();
}

void RectNode::setSize(const DPoint& pt) 
{
    m_Rect.setWidth(pt.x);
    m_Rect.setHeight(pt.y);
    setDrawNeeded(false);
}

const vector<double>& RectNode::getTexCoords() const
{
    return m_TexCoords;
}

void RectNode::setTexCoords(const vector<double>& coords)
{
    if (coords.size() != 5) {
        throw(Exception(AVG_ERR_OUT_OF_RANGE, 
                "Number of texture coordinates for a rectangle must be 5."));
    }
    m_TexCoords = coords;
    setDrawNeeded(false);
}

double RectNode::getAngle() const
{
    return m_Angle;
}

void RectNode::setAngle(double angle)
{
    m_Angle = fmod(angle, 2*PI);
    setDrawNeeded(false);
}

int RectNode::getNumVertexes()
{
    return 4*4;
}

int RectNode::getNumIndexes()
{
    return 6*4;
}

int RectNode::getNumFillVertexes()
{
    return 4;
}

int RectNode::getNumFillIndexes()
{
    return 6;
}

void RectNode::calcVertexes(VertexArrayPtr& pVertexArray, Pixel32 color)
{
    DPoint pivot = m_Rect.tl+m_Rect.size()/2;

    DPoint p1 = m_Rect.tl;
    DPoint p2(m_Rect.tl.x, m_Rect.br.y);
    DPoint p3 = m_Rect.br;
    DPoint p4(m_Rect.br.x, m_Rect.tl.y);
    DPoint rp1 = rotate(p1, m_Angle, pivot); 
    DPoint rp2 = rotate(p2, m_Angle, pivot); 
    DPoint rp3 = rotate(p3, m_Angle, pivot); 
    DPoint rp4 = rotate(p4, m_Angle, pivot); 

    updateLineData(pVertexArray, color, rp1, rp2, m_TexCoords[0], m_TexCoords[1]);
    updateLineData(pVertexArray, color, rp3, rp4, m_TexCoords[2], m_TexCoords[3]);
    p1.x -= getStrokeWidth()/2;
    p2.x -= getStrokeWidth()/2;
    p3.x += getStrokeWidth()/2;
    p4.x += getStrokeWidth()/2;
    rp1 = rotate(p1, m_Angle, pivot); 
    rp2 = rotate(p2, m_Angle, pivot); 
    rp3 = rotate(p3, m_Angle, pivot); 
    rp4 = rotate(p4, m_Angle, pivot); 
    updateLineData(pVertexArray, color, rp2, rp3, m_TexCoords[1], m_TexCoords[2]);
    updateLineData(pVertexArray, color, rp4, rp1, m_TexCoords[3], m_TexCoords[4]);
}

void RectNode::calcFillVertexes(VertexArrayPtr& pVertexArray, Pixel32 color)
{
    DPoint pivot = m_Rect.tl+m_Rect.size()/2;

    DPoint p1 = m_Rect.tl;
    DPoint p2(m_Rect.tl.x, m_Rect.br.y);
    DPoint p3 = m_Rect.br;
    DPoint p4(m_Rect.br.x, m_Rect.tl.y);
    DPoint rp1 = rotate(p1, m_Angle, pivot); 
    DPoint rp2 = rotate(p2, m_Angle, pivot); 
    DPoint rp3 = rotate(p3, m_Angle, pivot); 
    DPoint rp4 = rotate(p4, m_Angle, pivot); 
    pVertexArray->appendPos(rp1, DPoint(0,0), color);
    pVertexArray->appendPos(rp2, DPoint(0,1), color);
    pVertexArray->appendPos(rp3, DPoint(1,1), color);
    pVertexArray->appendPos(rp4, DPoint(1,0), color);
    pVertexArray->appendQuadIndexes(1, 0, 2, 3);
}

}
