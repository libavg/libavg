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
    vector<DPoint> v;
    vector<double> vd;
    return NodeDefinition("polygon", Node::buildNode<PolygonNode>)
        .extendDefinition(FilledVectorNode::createDefinition())
        .addArg(Arg<string>("linejoin", "bevel"))
        .addArg(Arg<vector<DPoint> >("pos", v, false, offsetof(PolygonNode, m_Pts)))
        .addArg(Arg<vector<double> >("texcoords", vd, false,
                offsetof(PolygonNode, m_TexCoords)))
        ;
}

PolygonNode::PolygonNode(const ArgList& args)
    : FilledVectorNode(args)
{
    args.setMembers(this);
    if (m_TexCoords.size() > m_Pts.size()+1) {
        throw(Exception(AVG_ERR_OUT_OF_RANGE, 
                "Too many texture coordinates in polygon"));
    }
    setLineJoin(args.getArgVal<string>("linejoin"));
    calcPolyLineCumulDist(m_CumulDist, m_Pts, true);
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
    setDrawNeeded();
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
    setDrawNeeded();
}

string PolygonNode::getLineJoin() const
{
    return lineJoin2String(m_LineJoin);
}

void PolygonNode::setLineJoin(const string& s)
{
    m_LineJoin = string2LineJoin(s);
    setDrawNeeded();
}

VisibleNodePtr PolygonNode::getElementByPos(const DPoint& pos)
{
    if (reactsToMouseEvents() && pointInPolygon(pos, m_Pts)) {
        return getVThis();
    } else {
        return VisibleNodePtr();
    }
}

void PolygonNode::calcVertexes(VertexArrayPtr& pVertexArray, Pixel32 color)
{
    if (getNumDifferentPts(m_Pts) < 3) {
        return;
    }
    if (m_EffTexCoords.empty()) {
        calcEffPolyLineTexCoords(m_EffTexCoords, m_TexCoords, m_CumulDist);
    }
    calcPolyLine(m_Pts, m_EffTexCoords, true, m_LineJoin, pVertexArray, color);
}

void PolygonNode::calcFillVertexes(VertexArrayPtr& pVertexArray, Pixel32 color)
{
    if (getNumDifferentPts(m_Pts) < 3) {
        return;
    }
    if (color.getA() > 0 && m_Pts.size() > 2) {
        DPoint minCoord = m_Pts[0];
        DPoint maxCoord = m_Pts[0];
        for (unsigned i=1; i<m_Pts.size(); ++i) {
            if (m_Pts[i].x < minCoord.x) {
                minCoord.x = m_Pts[i].x;
            }
            if (m_Pts[i].x > maxCoord.x) {
                maxCoord.x = m_Pts[i].x;
            }
            if (m_Pts[i].y < minCoord.y) {
                minCoord.y = m_Pts[i].y;
            }
            if (m_Pts[i].y > maxCoord.y) {
                maxCoord.y = m_Pts[i].y;
            }
        }
        vector<int> triIndexes;
        triangulatePolygon(m_Pts, triIndexes);
        for (unsigned i = 0; i < m_Pts.size(); ++i) {
            DPoint texCoord = calcFillTexCoord(m_Pts[i], minCoord, maxCoord);
            pVertexArray->appendPos(m_Pts[i], texCoord, color);
        }
        for (unsigned i = 0; i < triIndexes.size(); i+=3) {
            pVertexArray->appendTriIndexes(triIndexes[i], triIndexes[i+1], 
                    triIndexes[i+2]);
        }
    }
}

}
