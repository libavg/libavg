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
    double texCoords[] = {0, 0.25, 0.5, 0.75, 1};
    return NodeDefinition("rect", Node::buildNode<RectNode>)
        .extendDefinition(FilledVectorNode::createDefinition())
        .addArg(Arg<DPoint>("pos", DPoint(0,0), false, offsetof(RectNode, m_Rect.tl)))
        .addArg(Arg<DPoint>("size", DPoint(0,0)))
        .addArg(Arg<double>("angle", 0.0, false, offsetof(RectNode, m_Angle)))
        .addArg(Arg<vector<double> >("texcoords", vectorFromCArray(5, texCoords), false,
                offsetof(RectNode, m_TexCoords)))
        ;
}

RectNode::RectNode(const ArgList& args)
    : FilledVectorNode(args)
{
    args.setMembers(this);
    setSize(args.getArgVal<DPoint>("size"));
}

RectNode::~RectNode()
{
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
    setDrawNeeded();
}

DPoint RectNode::getSize() const 
{
    return m_Rect.size();
}

void RectNode::setSize(const DPoint& pt) 
{
    m_Rect.setWidth(pt.x);
    m_Rect.setHeight(pt.y);
    setDrawNeeded();
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
    setDrawNeeded();
}

double RectNode::getAngle() const
{
    return m_Angle;
}

void RectNode::setAngle(double angle)
{
    m_Angle = fmod(angle, 2*PI);
    setDrawNeeded();
}

VisibleNodePtr RectNode::getElementByPos(const DPoint & pos)
{
    DPoint pivot = m_Rect.tl+m_Rect.size()/2;
    DPoint rpos = pos.getRotatedPivot(m_Angle, pivot);
    if (rpos.x >= m_Rect.tl.x && rpos.y >= m_Rect.tl.y && rpos.x < m_Rect.br.x && 
            rpos.y < m_Rect.br.y && reactsToMouseEvents())
    {
        return getVThis();
    } else {
        return VisibleNodePtr();
    }
}

void RectNode::calcVertexes(VertexArrayPtr& pVertexArray, Pixel32 color)
{
    DPoint pivot = m_Rect.tl+m_Rect.size()/2;

    DPoint p1 = m_Rect.tl;
    DPoint p2(m_Rect.tl.x, m_Rect.br.y);
    DPoint p3 = m_Rect.br;
    DPoint p4(m_Rect.br.x, m_Rect.tl.y);
    
    vector<DPoint> pts; 
    pts.push_back(p1.getRotatedPivot(m_Angle, pivot));
    pts.push_back(p2.getRotatedPivot(m_Angle, pivot));
    pts.push_back(p3.getRotatedPivot(m_Angle, pivot));
    pts.push_back(p4.getRotatedPivot(m_Angle, pivot));
    calcPolyLine(pts, m_TexCoords, true, LJ_MITER, pVertexArray, color);
}

void RectNode::calcFillVertexes(VertexArrayPtr& pVertexArray, Pixel32 color)
{
    DPoint pivot = m_Rect.tl+m_Rect.size()/2;

    DPoint p1 = m_Rect.tl;
    DPoint p2(m_Rect.tl.x, m_Rect.br.y);
    DPoint p3 = m_Rect.br;
    DPoint p4(m_Rect.br.x, m_Rect.tl.y);
    DPoint rp1 = p1.getRotatedPivot(m_Angle, pivot);
    DPoint rp2 = p2.getRotatedPivot(m_Angle, pivot);
    DPoint rp3 = p3.getRotatedPivot(m_Angle, pivot);
    DPoint rp4 = p4.getRotatedPivot(m_Angle, pivot);
    pVertexArray->appendPos(rp1, getFillTexCoord1(), color);
    DPoint blTexCoord = DPoint(getFillTexCoord1().x, getFillTexCoord2().y);
    pVertexArray->appendPos(rp2, blTexCoord, color);
    pVertexArray->appendPos(rp3, getFillTexCoord2(), color);
    DPoint trTexCoord = DPoint(getFillTexCoord2().x, getFillTexCoord1().y);
    pVertexArray->appendPos(rp4, trTexCoord, color);
    pVertexArray->appendQuadIndexes(1, 0, 2, 3);
}

}
