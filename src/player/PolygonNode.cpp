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

#include "PolygonNode.h"

#include "NodeDefinition.h"

#include "../graphics/VertexArray.h"
#include "../base/Exception.h"
#include "../base/GeomHelper.h"
#include "../base/Triangulate.h"

#include <iostream>
#include <sstream>

using namespace std;

namespace avg {

NodeDefinition PolygonNode::createDefinition()
{
    return NodeDefinition("polygon", Node::buildNode<PolygonNode>)
        .extendDefinition(PolyLineNode::createDefinition())
        .addArg(Arg<double>("fillopacity", 0, false, 
                offsetof(PolygonNode, m_FillOpacity)))
        .addArg(Arg<string>("fillcolor", "FFFFFF", false, 
                offsetof(PolygonNode, m_sFillColorName)))
        ;
}

PolygonNode::PolygonNode(const ArgList& Args, bool bFromXML)
{
    Args.setMembers(this);
    setLineJoin(Args.getArgVal<string>("linejoin"));
    m_FillColor = colorStringToColor(m_sFillColorName);
}

PolygonNode::~PolygonNode()
{
}

double PolygonNode::getFillOpacity() const
{
    return m_FillOpacity;
}

void PolygonNode::setFillOpacity(double opacity)
{
    m_FillOpacity = opacity;
    setDrawNeeded(true);
}

void PolygonNode::setFillColor(const string& sFillColor)
{
    if (m_sFillColorName != sFillColor) {
        m_sFillColorName = sFillColor;
        m_FillColor = colorStringToColor(m_sFillColorName);
        setDrawNeeded(false);
    }
}

const string& PolygonNode::getFillColor() const
{
    return m_sFillColorName;
}

int PolygonNode::getNumVertexes()
{
    const vector<DPoint>& pts = getPos();
    if (pts.size() < 3) {
        return 0;
    }
    int numVerts;
    switch(getLineJoinEnum()) {
        case LJ_MITER:
            numVerts = 2*pts.size();
            break;
        case LJ_BEVEL:
            numVerts = 3*pts.size();
            break;
        default:
            assert(false);
    }
    if (m_FillOpacity > 0.001) {
        numVerts += pts.size();
    }
    return numVerts;
}

int PolygonNode::getNumIndexes()
{
    const vector<DPoint>& pts = getPos();
    if (pts.size() < 3) {
        return 0;
    }
    int numIndexes;
    switch(getLineJoinEnum()) {
        case LJ_MITER:
            numIndexes = 6*pts.size();
            break;
        case LJ_BEVEL:
            numIndexes = 9*pts.size();
            break;
        default:
            assert(false);
    }
    if (m_FillOpacity > 0.001) {
        numIndexes += (pts.size()-2)*3;
    }
    return numIndexes;
}

void PolygonNode::calcVertexes(VertexDataPtr& pVertexData, double opacity)
{
    const vector<DPoint>& pts = getPos();
    if (pts.size() < 3) {
        return;
    }
    int numPts = pts.size();
    int startOutlinePt = 0;
    int startOutlineIndex = 0;
    double curOpacity = opacity*getOpacity();
    Pixel32 color = getColorVal();
    color.setA((unsigned char)(curOpacity*255));

    // Fill
    if (m_FillOpacity > 0.001) {
        double curOpacity = opacity*m_FillOpacity;
        Pixel32 fillColor = m_FillColor;
        fillColor.setA((unsigned char)(curOpacity*255));

        vector<int> triIndexes;
        triangulatePolygon(pts, triIndexes);
        for (int i=0; i<numPts; ++i) {
            pVertexData->appendPos(pts[i], DPoint(0,0), fillColor);
        }
        startOutlinePt = numPts;
        for (unsigned int i=0; i<triIndexes.size(); i+=3) {
            pVertexData->appendTriIndexes(triIndexes[i], triIndexes[i+1], triIndexes[i+2]);
        }
        startOutlineIndex = triIndexes.size();
    }

    // Outline
    vector<WideLine> lines;
    lines.reserve(numPts);
    for (int i=0; i<numPts-1; ++i) {
        lines.push_back(WideLine(pts[i], pts[i+1], getStrokeWidth()));
    }
    lines.push_back(WideLine(pts[numPts-1], pts[0], getStrokeWidth()));

    const WideLine* pLastLine = &(lines[numPts-1]);

    for (int i=0; i<numPts; ++i) {
        const WideLine* pThisLine = &(lines[i]);
        DPoint pli = getLineLineIntersection(pLastLine->pl0, pLastLine->dir, 
                pThisLine->pl0, pThisLine->dir);
        DPoint pri = getLineLineIntersection(pLastLine->pr0, pLastLine->dir, 
                pThisLine->pr0, pThisLine->dir);
        int curVertex = pVertexData->getCurVert();
        switch(getLineJoinEnum()) {
            case LJ_MITER:
                pVertexData->appendPos(pli, DPoint(0,0), color);
                pVertexData->appendPos(pri, DPoint(0,0), color);
                pLastLine = pThisLine;
                if (i<numPts-1) {
                    pVertexData->appendQuadIndexes(
                            curVertex+1, curVertex, curVertex+3, curVertex+2);
                } else {
                    pVertexData->appendQuadIndexes(
                            curVertex+1, curVertex, startOutlinePt+1, startOutlinePt);
                }
                break;
            case LJ_BEVEL:
                {
                    Triangle tri(pLastLine->pl1, pThisLine->pl0, pri);
                    if (tri.isClockwise()) {
                        pVertexData->appendPos(pri, DPoint(0,0), color);
                        pVertexData->appendPos(pLastLine->pl1, DPoint(0,0), color);
                        pVertexData->appendPos(pThisLine->pl0, DPoint(0,0), color);
                        pVertexData->appendTriIndexes(
                                curVertex, curVertex+1, curVertex+2);
                        if (i<numPts-1) {
                            pVertexData->appendQuadIndexes(
                                    curVertex, curVertex+2, curVertex+3, curVertex+4);
                        } else {
                            pVertexData->appendQuadIndexes(curVertex,
                                    curVertex+2, startOutlinePt, startOutlinePt+1);
                        }
                    } else {
                        pVertexData->appendPos(pLastLine->pr1, DPoint(0,0), color);
                        pVertexData->appendPos(pli, DPoint(0,0), color);
                        pVertexData->appendPos(pThisLine->pr0, DPoint(0,0), color);
                        pVertexData->appendTriIndexes(
                                curVertex, curVertex+1, curVertex+2);
                        if (i<numPts-1) {
                            pVertexData->appendQuadIndexes(
                                    curVertex+2, curVertex+1, curVertex+3, curVertex+4);
                        } else {
                            pVertexData->appendQuadIndexes(curVertex+2, 
                                    curVertex+1, startOutlinePt, startOutlinePt+1);
                        }
                    }
                }
                break;
            default:
                assert(false);
        }
        pLastLine = pThisLine;
    }
}

}
