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
    : VectorNode(Args)
{
    Args.setMembers(this);
    setLineJoin(Args.getArgVal<string>("linejoin"));
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

string PolyLineNode::getLineJoin() const
{
    switch(m_LineJoin) {
        case LJ_MITER:
            return "miter";
        case LJ_BEVEL:
            return "bevel";
        default:
            assert(false);
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
            return 9*(m_Pts.size()-1)-1;
        default:
            assert(false);
    }
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

    int curVertex = 0;
    int curIndex = 0;
    pVertexData->setPos(curVertex++, lines[0].pl0, DPoint(0,0), color);
    pVertexData->setPos(curVertex++, lines[0].pr0, DPoint(0,0), color);
    for (int i=0; i<numPts-2; ++i) {
        const WideLine& line1 = lines[i];
        const WideLine& line2 = lines[i+1];
        DPoint pli = getLineLineIntersection(line1.pl0, line1.dir, line2.pl0, line2.dir);
        DPoint pri = getLineLineIntersection(line1.pr0, line1.dir, line2.pr0, line2.dir);

        switch (m_LineJoin) {
            case LJ_MITER:
                pVertexData->setPos(curVertex, pli, DPoint(0,0), color);
                pVertexData->setPos(curVertex+1, pri, DPoint(0,0), color);
                
                pVertexData->setTriIndexes(curIndex, curVertex-2, curVertex-1, curVertex+1);
                pVertexData->setTriIndexes(curIndex+3, curVertex-2, curVertex+1, curVertex);
                curVertex += 2;
                curIndex += 6;
                break;
            case LJ_BEVEL:
                {
                    Triangle tri(line1.pl1, line2.pl0, pri);
                    if (tri.getArea() < 0) {
                        pVertexData->setPos(curVertex, line1.pl1, DPoint(0,0), color);
                        pVertexData->setPos(curVertex+1, line2.pl0, DPoint(0,0), color);
                        pVertexData->setPos(curVertex+2, pri, DPoint(0,0), color);
                        pVertexData->setTriIndexes(curIndex, 
                                curVertex-2, curVertex-1, curVertex+2);
                        pVertexData->setTriIndexes(curIndex+3, 
                                curVertex-2, curVertex+2, curVertex);
                        pVertexData->setTriIndexes(curIndex+6, 
                                curVertex, curVertex+1, curVertex+2);
                    } else {
                        pVertexData->setPos(curVertex, line1.pr1, DPoint(0,0), color);
                        pVertexData->setPos(curVertex+1, pli, DPoint(0,0), color);
                        pVertexData->setPos(curVertex+2, line2.pr0, DPoint(0,0), color);
                        pVertexData->setTriIndexes(curIndex, 
                                curVertex-2, curVertex-1, curVertex+1);
                        pVertexData->setTriIndexes(curIndex+3, 
                                curVertex-1, curVertex+1, curVertex);
                        pVertexData->setTriIndexes(curIndex+6, 
                                curVertex, curVertex+1, curVertex+2);
                    }
                    curVertex += 3;
                    curIndex += 9;
                }
                break;
            default:
                assert(false);
        }
    }
    pVertexData->setPos(curVertex, lines[numPts-2].pl1, DPoint(0,0), color);
    pVertexData->setPos(curVertex+1, lines[numPts-2].pr1, DPoint(0,0), color);
    pVertexData->setTriIndexes(curIndex, curVertex-2, curVertex-1, curVertex+1);
    pVertexData->setTriIndexes(curIndex+3, curVertex-2, curVertex+1, curVertex);
}

}
