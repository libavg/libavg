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
#include "../base/Triangle.h"

#include <iostream>
#include <sstream>

using namespace std;

namespace avg {

NodeDefinition PolyLineNode::createDefinition()
{
    return NodeDefinition("polyline", Node::buildNode<PolyLineNode>)
        .extendDefinition(VectorNode::createDefinition())
        .addArg(Arg<string>("linejoin", "bevel"))
        ;
}

PolyLineNode::PolyLineNode(const ArgList& Args, bool bFromXML)
{
    Args.setMembers(this);
    setLineJoin(Args.getArgVal<string>("linejoin"));
}

PolyLineNode::PolyLineNode()
{
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
    m_Pts.clear();
    m_Pts.reserve(pts.size());
    if (!pts.empty()) {
        m_Pts.push_back(pts[0]);
        // Remove possible duplicated points.
        for (unsigned int i=1; i<pts.size(); ++i) {
            if (pts[i] != pts[i-1]) {
                m_Pts.push_back(pts[i]);
            }
        }
    }
    setDrawNeeded(true);
}

string PolyLineNode::getLineJoin() const
{
    switch(m_LineJoin) {
        case LJ_MITER:
            return "miter";
        case LJ_BEVEL:
            return "bevel";
        default:
            assert(false);
            return 0;
    }
}

void PolyLineNode::setLineJoin(const string& sAlign)
{
    if (sAlign == "miter") {
        m_LineJoin = LJ_MITER;
    } else if (sAlign == "bevel") {
        m_LineJoin = LJ_BEVEL;
    } else {
        throw(Exception(AVG_ERR_UNSUPPORTED, 
                "Vector linejoin "+sAlign+" not supported."));
    }
    setDrawNeeded(true);
}

int PolyLineNode::getNumVertexes()
{
    if (m_Pts.size() < 2) {
        return 0;
    }
    switch (m_LineJoin) {
        case LJ_MITER:
            return 2*m_Pts.size();
        case LJ_BEVEL:
            return 3*m_Pts.size()-2;
        default:
            assert(false);
            return 0;
    }

}

int PolyLineNode::getNumIndexes()
{
    if (m_Pts.size() < 2) {
        return 0;
    }
    switch (m_LineJoin) {
        case LJ_MITER:
            return 6*(m_Pts.size()-1);
        case LJ_BEVEL:
            return 3*(3*m_Pts.size()-4);
        default:
            assert(false);
            return 0;
    }
}

void PolyLineNode::calcVertexes(VertexDataPtr& pVertexData, double opacity)
{
    if (m_Pts.size() < 2) {
        return;
    }
    double curOpacity = opacity*getOpacity();
    Pixel32 color = getColorVal();
    color.setA((unsigned char)(curOpacity*255));
    
    int numPts = m_Pts.size();

    vector<WideLine> lines;
    lines.reserve(numPts-1);
    for (int i=0; i<numPts-1; ++i) {
        lines.push_back(WideLine(m_Pts[i], m_Pts[i+1], getStrokeWidth()));
    }

    pVertexData->appendPos(lines[0].pl0, DPoint(0,0), color);
    pVertexData->appendPos(lines[0].pr0, DPoint(0,0), color);
    for (int i=0; i<numPts-2; ++i) {
        const WideLine& line1 = lines[i];
        const WideLine& line2 = lines[i+1];
        DPoint pli = getLineLineIntersection(line1.pl0, line1.dir, line2.pl0, line2.dir);
        DPoint pri = getLineLineIntersection(line1.pr0, line1.dir, line2.pr0, line2.dir);

        int curVertex = pVertexData->getCurVert();
        switch (m_LineJoin) {
            case LJ_MITER:
                pVertexData->appendPos(pli, DPoint(0,0), color);
                pVertexData->appendPos(pri, DPoint(0,0), color);
                pVertexData->appendQuadIndexes(
                        curVertex-1, curVertex-2, curVertex+1, curVertex);
                break;
            case LJ_BEVEL:
                {
                    Triangle tri(line1.pl1, line2.pl0, pri);
                    if (tri.isClockwise()) {
                        pVertexData->appendPos(line1.pl1, DPoint(0,0), color);
                        pVertexData->appendPos(line2.pl0, DPoint(0,0), color);
                        pVertexData->appendPos(pri, DPoint(0,0), color);
                        pVertexData->appendQuadIndexes(
                                curVertex-1, curVertex-2, curVertex+2, curVertex);
                        pVertexData->appendTriIndexes(
                                curVertex, curVertex+1, curVertex+2);
                    } else {
                        pVertexData->appendPos(line1.pr1, DPoint(0,0), color);
                        pVertexData->appendPos(pli, DPoint(0,0), color);
                        pVertexData->appendPos(line2.pr0, DPoint(0,0), color);
                        pVertexData->appendQuadIndexes(
                                curVertex-2, curVertex-1, curVertex+1, curVertex);
                        pVertexData->appendTriIndexes(
                                curVertex, curVertex+1, curVertex+2);
                    }
                }
                break;
            default:
                assert(false);
        }
    }
    int curVertex = pVertexData->getCurVert();
    pVertexData->appendPos(lines[numPts-2].pl1, DPoint(0,0), color);
    pVertexData->appendPos(lines[numPts-2].pr1, DPoint(0,0), color);
    pVertexData->appendQuadIndexes(curVertex-1, curVertex-2, curVertex+1, curVertex);
}

PolyLineNode::LineJoin PolyLineNode::getLineJoinEnum() const
{
    return m_LineJoin;
}

}
