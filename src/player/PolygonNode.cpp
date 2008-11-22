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
    int numVerts = 2*pts.size();
    if (m_FillOpacity > 0.001) {
        numVerts += pts.size();
    }
    return numVerts;
}

int PolygonNode::getNumIndexes()
{
    const vector<DPoint>& pts = getPos();
    if (pts.size() < 2) {
        return 0;
    }
    int numIndexes = 6*pts.size();
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
    Pixel32 color = getColorVal();

    // Fill
    if (m_FillOpacity > 0.001) {
        double curOpacity = opacity*m_FillOpacity;
        Pixel32 fillColor = m_FillColor;
        fillColor.setA((unsigned char)(curOpacity*255));

        vector<int> triIndexes;
        triangulatePolygon(pts, triIndexes);
        for (int i=0; i<numPts; ++i) {
            pVertexData->setPos(i, pts[i], DPoint(0,0), fillColor);
        }
        startOutlinePt = numPts;
        for (unsigned int i=0; i<triIndexes.size(); ++i) {
            pVertexData->setIndex(i, triIndexes[i]);
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
        pVertexData->setPos(startOutlinePt+2*i, pli, DPoint(0,0), color);
        pVertexData->setPos(startOutlinePt+2*i+1, pri, DPoint(0,0), color);
        pLastLine = pThisLine;
    }

    for (int i=0; i<numPts-1; ++i) {
        int loopIndex = startOutlineIndex+i*6;
        int loopVertex = startOutlinePt+i*2;
        pVertexData->setTriIndexes(loopIndex, loopVertex, loopVertex+1, loopVertex+3);
        pVertexData->setTriIndexes(loopIndex+3, loopVertex, loopVertex+3, loopVertex+2);
    }
    int loopIndex = startOutlineIndex+(numPts-1)*6;
    int loopVertex = startOutlinePt+(numPts-1)*2;
    pVertexData->setTriIndexes(loopIndex, loopVertex, loopVertex+1, startOutlinePt+1);
    pVertexData->setTriIndexes(loopIndex+3, loopVertex, startOutlinePt, startOutlinePt+1);
}

}
