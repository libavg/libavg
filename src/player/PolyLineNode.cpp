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
    
    int numPts = m_Pts.size();

    vector<WideLine> lines;
    lines.reserve(numPts-1);
    for (int i=0; i<numPts-1; ++i) {
        lines.push_back(WideLine(m_Pts[i], m_Pts[i+1], getStrokeWidth()));
    }

    pVertexData->setPos(0, lines[0].pl0, DPoint(0,0), color);
    pVertexData->setPos(1, lines[0].pr0, DPoint(0,0), color);
    for (int i=0; i<numPts-2; ++i) {
        const WideLine& line1 = lines[i];
        const WideLine& line2 = lines[i+1];
        DPoint pli = getLineLineIntersection(line1.pl0, line1.dir, line2.pl0, line2.dir);
        DPoint pri = getLineLineIntersection(line1.pr0, line1.dir, line2.pr0, line2.dir);

        pVertexData->setPos(2*i+2, pli, DPoint(0,0), color);
        pVertexData->setPos(2*i+3, pri, DPoint(0,0), color);
    }
    pVertexData->setPos((numPts-1)*2, lines[numPts-2].pl1, DPoint(0,0), color);
    pVertexData->setPos((numPts-1)*2+1, lines[numPts-2].pr1, DPoint(0,0), color);

    for (int i=0; i<numPts-1; ++i) {
        int loopIndex = i*6;
        int loopVertex = i*2;
        pVertexData->setTriIndexes(loopIndex, loopVertex, loopVertex+1, loopVertex+3);
        pVertexData->setTriIndexes(loopIndex+3, loopVertex, loopVertex+3, loopVertex+2);
    }
}

}
