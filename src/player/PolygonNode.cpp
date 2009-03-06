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
        .extendDefinition(FilledVectorNode::createDefinition())
        .addArg(Arg<string>("linejoin", "bevel"))
        ;
}

PolygonNode::PolygonNode(const ArgList& Args, bool bFromXML)
    : FilledVectorNode(Args)
{
    Args.setMembers(this);
    setLineJoin(Args.getArgVal<string>("linejoin"));
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
    m_Pts = pts;
    m_TexCoords.clear();
    m_EffTexCoords.clear();
    calcPolyLineCumulDist(m_CumulDist, m_Pts, true);
    setDrawNeeded(true);
}
        
const vector<double>& PolygonNode::getTexCoords() const
{
    return m_TexCoords;
}

void PolygonNode::setTexCoords(const vector<double>& coords)
{
    if (coords.size() > m_Pts.size()+1) {
        throw(Exception(AVG_ERR_OUT_OF_RANGE, 
                "Too many texture coordinates in polygon"));
    }
    m_EffTexCoords.clear();
    m_TexCoords = coords;
    setDrawNeeded(false);
}

string PolygonNode::getLineJoin() const
{
    return lineJoin2String(m_LineJoin);
}

void PolygonNode::setLineJoin(const string& s)
{
    m_LineJoin = string2LineJoin(s);
    setDrawNeeded(true);
}

int PolygonNode::getNumVertexes()
{
    if (m_Pts.size() < 3) {
        return 0;
    }
    int numPts = m_Pts.size();
    for (unsigned i=1; i<m_Pts.size(); ++i) {
        if (calcDistSquared(m_Pts[i], m_Pts[i-1])<0.1) {
            numPts--;
        }
    }
    int numVerts;
    switch(m_LineJoin) {
        case LJ_MITER:
            numVerts = 2*numPts+2;
            break;
        case LJ_BEVEL:
            numVerts = 3*numPts+2;
            break;
        default:
            assert(false);
    }
    return numVerts;
}

int PolygonNode::getNumIndexes()
{
    if (m_Pts.size() < 3) {
        return 0;
    }
    int numPts = m_Pts.size();
    for (unsigned i=1; i<m_Pts.size(); ++i) {
        if (calcDistSquared(m_Pts[i], m_Pts[i-1])<0.1) {
            numPts--;
        }
    }
    int numIndexes;
    switch(m_LineJoin) {
        case LJ_MITER:
            numIndexes = 6*numPts;
            break;
        case LJ_BEVEL:
            numIndexes = 9*numPts;
            break;
        default:
            assert(false);
    }
    return numIndexes;
}

int PolygonNode::getNumFillVertexes()
{

    if (getFillOpacity() < 0.001 || m_Pts.size() < 3) {
        return 0;
    } else {
        return m_Pts.size();
    }
}

int PolygonNode::getNumFillIndexes()
{
    if (getFillOpacity() < 0.001 || m_Pts.size() < 3) {
        return 0;
    } else {
        return (m_Pts.size()-2)*3;
    }

}

void PolygonNode::calcVertexes(VertexArrayPtr& pVertexArray, Pixel32 color)
{
    if (m_Pts.size() < 3) {
        return;
    }
    if (m_EffTexCoords.empty()) {
        calcEffPolyLineTexCoords(m_EffTexCoords, m_TexCoords, m_CumulDist);
    }
    calcPolyLine(m_Pts, m_EffTexCoords, true, m_LineJoin, pVertexArray, color);
}

void PolygonNode::calcFillVertexes(VertexArrayPtr& pVertexArray, Pixel32 color)
{
    if (color.getA() > 0 && m_Pts.size() > 2) {
        vector<int> triIndexes;
        triangulatePolygon(m_Pts, triIndexes);
        for (unsigned i=0; i<m_Pts.size(); ++i) {
            pVertexArray->appendPos(m_Pts[i], DPoint(0,0), color);
        }
        for (unsigned i=0; i<triIndexes.size(); i+=3) {
            pVertexArray->appendTriIndexes(triIndexes[i], triIndexes[i+1], triIndexes[i+2]);
        }
    }
}

}
