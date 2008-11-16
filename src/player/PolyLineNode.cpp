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

#include "PolyLineNode.h"

#include "NodeDefinition.h"

#include "../graphics/VertexArray.h"
#include "../base/Exception.h"
#include "../base/GeomHelper.h"

#include <iostream>
#include <sstream>

using namespace std;

namespace avg {

NodeDefinition PolyLineNode::createDefinition()
{
    return NodeDefinition("polyline", Node::buildNode<PolyLineNode>)
        .extendDefinition(VectorNode::createDefinition());
}

PolyLineNode::PolyLineNode(const ArgList& Args, bool bFromXML)
    : VectorNode(Args)
{
    Args.setMembers(this);
}

PolyLineNode::~PolyLineNode()
{
}

const vector<DPoint>& PolyLineNode::getPos() const 
{
    return m_Pts;
}

void PolyLineNode::setPos(const vector<DPoint>& pts) 
{
    m_Pts = pts;
    setDrawNeeded(true);
}

int PolyLineNode::getNumVertexes()
{
    if (m_Pts.size() < 2) {
        return 0;
    }
    return 2*m_Pts.size();
}

int PolyLineNode::getNumIndexes()
{
    if (m_Pts.size() < 2) {
        return 0;
    }
    return 6*(m_Pts.size()-1);
}

void PolyLineNode::calcVertexes(VertexDataPtr& pVertexData, double opacity)
{
    if (m_Pts.size() < 2) {
        return;
    }
    Pixel32 color = getColorVal();
    // Generate all points.
    int numPts = m_Pts.size();
    DPoint w1;
    DPoint w2(getLineWidthOffset(m_Pts[0], m_Pts[1]));
    DPoint pl1;
    DPoint pl2 = m_Pts[0]-w2;
    DPoint pr1;
    DPoint pr2 = m_Pts[0]+w2;
    w2 = DPoint(w2.y, -w2.x); // Point in the direction of the line, not 
                              // perpendicular to it.

    pVertexData->setPos(0, pl2, DPoint(0,0), color);
    pVertexData->setPos(1, pr2, DPoint(0,0), color);
    for (int i=1; i<numPts-1; ++i) {
        w1 = w2;
        pl1 = pl2;
        pr1 = pr2;
        w2 = getLineWidthOffset(m_Pts[i], m_Pts[i+1]);
        pl2 = m_Pts[i]-w2;
        pr2 = m_Pts[i]+w2;
        w2 = DPoint(w2.y, -w2.x);
        DPoint pli = getLineLineIntersection(pl1, w1, pl2, w2);
        DPoint pri = getLineLineIntersection(pr1, w1, pr2, w2);

        pVertexData->setPos(2*i, pli, DPoint(0,0), color);
        pVertexData->setPos(2*i+1, pri, DPoint(0,0), color);
    }
    DPoint w = getLineWidthOffset(m_Pts[numPts-2], m_Pts[numPts-1]);
    pVertexData->setPos((numPts-1)*2, m_Pts[numPts-1]-w, DPoint(0,0), 
            color);
    pVertexData->setPos((numPts-1)*2+1, m_Pts[numPts-1]+w, DPoint(0,0), 
            color);

    // Generate the vertices.
    for (int i=0; i<numPts-1; ++i) {
        int loopIndex = i*6;
        int loopVertex = i*2;
        pVertexData->setIndex(loopIndex, loopVertex);
        pVertexData->setIndex(loopIndex+1, loopVertex+1);
        pVertexData->setIndex(loopIndex+2, loopVertex+3);
        pVertexData->setIndex(loopIndex+3, loopVertex);
        pVertexData->setIndex(loopIndex+4, loopVertex+3);
        pVertexData->setIndex(loopIndex+5, loopVertex+2);
    }
}

}
