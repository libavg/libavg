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

RectNode::RectNode(const ArgList& Args, bool bFromXML)
    : FilledVectorNode(Args)
{
    Args.setMembers(this);
    m_Rect.setSize(Args.getArgVal<DPoint>("size"));
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

NodePtr RectNode::getElementByPos(const DPoint & pos)
{
    DPoint pivot = m_Rect.tl+m_Rect.size()/2;
    DPoint rpos = pos.getRotatedPivot(m_Angle, pivot);
    if (rpos.x >= m_Rect.tl.x && rpos.y >= m_Rect.tl.y && rpos.x < m_Rect.br.x && 
            rpos.y < m_Rect.br.y && reactsToMouseEvents())
    {
        return getThis();
    } else {
        return NodePtr();
    }
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
    DPoint rp1 = p1.getRotatedPivot(m_Angle, pivot);
    DPoint rp2 = p2.getRotatedPivot(m_Angle, pivot);
    DPoint rp3 = p3.getRotatedPivot(m_Angle, pivot);
    DPoint rp4 = p4.getRotatedPivot(m_Angle, pivot);

    double width = getStrokeWidth();
    pVertexArray->addLineData(color, rp1, rp2, width, m_TexCoords[0], m_TexCoords[1]);
    pVertexArray->addLineData(color, rp3, rp4, width, m_TexCoords[2], m_TexCoords[3]);
    p1.x -= width/2;
    p2.x -= width/2;
    p3.x += width/2;
    p4.x += width/2;
    rp1 = p1.getRotatedPivot(m_Angle, pivot);
    rp2 = p2.getRotatedPivot(m_Angle, pivot);
    rp3 = p3.getRotatedPivot(m_Angle, pivot);
    rp4 = p4.getRotatedPivot(m_Angle, pivot);
    pVertexArray->addLineData(color, rp2, rp3, width, m_TexCoords[1], m_TexCoords[2]);
    pVertexArray->addLineData(color, rp4, rp1, width, m_TexCoords[3], m_TexCoords[4]);
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
