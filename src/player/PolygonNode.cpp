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
        .extendDefinition(VectorNode::createDefinition())
        .addArg(Arg<double>("fillopacity", 0, false, 
                offsetof(PolygonNode, m_FillOpacity)))
        .addArg(Arg<string>("fillcolor", "FFFFFF", false, 
                offsetof(PolygonNode, m_sFillColorName)))
        ;
}

PolygonNode::PolygonNode(const ArgList& Args, bool bFromXML)
    : VectorNode(Args)
{
    Args.setMembers(this);
    m_FillColor = colorStringToColor(m_sFillColorName);
}

PolygonNode::~PolygonNode()
{
}

const vector<DPoint>& PolygonNode::getPos() const 
{
    return m_Pts;
}

void PolygonNode::setPos(const vector<DPoint>& pts) 
{
    m_Pts.clear();
    m_Pts.push_back(pts[0]);
    // Remove possible duplicated points.
    for (unsigned int i=1; i<pts.size(); ++i) {
        if (pts[i] != pts[i-1]) {
            m_Pts.push_back(pts[i]);
        }
    }
    setDrawNeeded(true);
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
    if (m_Pts.size() < 3) {
        return 0;
    }
    int numVerts = 2*m_Pts.size();
    if (m_FillOpacity > 0.001) {
        numVerts += m_Pts.size();
    }
    return numVerts;
}

int PolygonNode::getNumIndexes()
{
    if (m_Pts.size() < 2) {
        return 0;
    }
    int numIndexes = 6*m_Pts.size();
    if (m_FillOpacity > 0.001) {
        numIndexes += (m_Pts.size()-2)*3;
    }
    return numIndexes;
}

void PolygonNode::calcVertexes(VertexDataPtr& pVertexData, double opacity)
{
    if (m_Pts.size() < 3) {
        return;
    }
    int numPts = m_Pts.size();
    int startOutlinePt = 0;
    int startOutlineIndex = 0;
    Pixel32 color = getColorVal();

    // Polygon
    if (m_FillOpacity > 0.001) {
        double curOpacity = opacity*m_FillOpacity;
        Pixel32 fillColor = m_FillColor;
        fillColor.setA((unsigned char)(curOpacity*255));

        vector<int> triIndexes;
        triangulatePolygon(m_Pts, triIndexes);
        for (int i=0; i<numPts; ++i) {
            pVertexData->setPos(i, m_Pts[i], DPoint(0,0), fillColor);
        }
        startOutlinePt = numPts;
        for (unsigned int i=0; i<triIndexes.size(); ++i) {
            pVertexData->setIndex(i, triIndexes[i]);
        }
        startOutlineIndex = triIndexes.size();
    }
    // Outline
    if (m_Pts.size() < 3) {
        return;
    }
    
    DPoint w1;
    DPoint w2(getLineWidthOffset(m_Pts[numPts-1], m_Pts[0]));
    DPoint pl1;
    DPoint pl2 = m_Pts[0]-w2;
    DPoint pr1;
    DPoint pr2 = m_Pts[0]+w2;
    w2 = DPoint(w2.y, -w2.x); // Point in the direction of the line, not 
                              // perpendicular to it.
    
    for (int i=0; i<numPts; ++i) {
        int nextPt = (i+1) % numPts;
        w1 = w2;
        pl1 = pl2;
        pr1 = pr2;
        w2 = getLineWidthOffset(m_Pts[i], m_Pts[nextPt]);
        pl2 = m_Pts[i]-w2;
        pr2 = m_Pts[i]+w2;
        w2 = DPoint(w2.y, -w2.x);
        DPoint pli = getLineLineIntersection(pl1, w1, pl2, w2);
        DPoint pri = getLineLineIntersection(pr1, w1, pr2, w2);

        pVertexData->setPos(startOutlinePt+2*i, pli, DPoint(0,0), color);
        pVertexData->setPos(startOutlinePt+2*i+1, pri, DPoint(0,0), color);
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
