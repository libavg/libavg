//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2014 Ulrich von Zadow
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

#include "TypeDefinition.h"

#include "../base/Exception.h"
#include "../base/GeomHelper.h"
#include "../base/triangulate/Triangulate.h"

#include "../glm/gtx/norm.hpp"

#include <iostream>
#include <sstream>

using namespace std;

namespace avg {

void PolygonNode::registerType()
{
    VectorVec2Vector cv;
    vector<glm::vec2> v;
    vector<float> vd;
    TypeDefinition def = TypeDefinition("polygon", "filledvectornode",
            ExportedObject::buildObject<PolygonNode>)
        .addArg(Arg<string>("linejoin", "bevel"))
        .addArg(Arg<vector<glm::vec2> >("pos", v, false, offsetof(PolygonNode, m_Pts)))
        .addArg(Arg<vector<float> >("texcoords", vd, false,
                offsetof(PolygonNode, m_TexCoords)))
        .addArg(Arg<VectorVec2Vector>("holes", cv, false, offsetof(PolygonNode, m_Holes)))
        ;
    TypeRegistry::get()->registerType(def);
}

PolygonNode::PolygonNode(const ArgList& args)
    : FilledVectorNode(args)
{
    args.setMembers(this);
    if (m_TexCoords.size() > m_Pts.size()+1) {
        throw(Exception(AVG_ERR_OUT_OF_RANGE, 
                "Too many texture coordinates in polygon"));
    }
    if (m_Pts.size() != 0 && m_Pts.size() < 3) {
        throw(Exception(AVG_ERR_UNSUPPORTED,
                "A polygon must have min. tree points."));
    }
    if (m_Holes.size() > 0) {
        for (unsigned int i = 0; i < m_Holes.size(); i++) {
            if (m_Holes[i].size() < 3) {
                throw(Exception(AVG_ERR_UNSUPPORTED,
                        "A hole of a polygon must have min. tree points."));
            }
        }
    }
    setLineJoin(args.getArgVal<string>("linejoin"));
    calcPolyLineCumulDist(m_CumulDist, m_Pts, true);
}

PolygonNode::~PolygonNode()
{
}

const vector<glm::vec2>& PolygonNode::getPos() const 
{
    return m_Pts;
}

void PolygonNode::setPos(const vector<glm::vec2>& pts) 
{
    m_Pts.clear();
    m_Pts = pts;
    m_TexCoords.clear();
    m_EffTexCoords.clear();
    calcPolyLineCumulDist(m_CumulDist, m_Pts, true);
    setDrawNeeded();
}
        
const vector<float>& PolygonNode::getTexCoords() const
{
    return m_TexCoords;
}

const VectorVec2Vector& PolygonNode::getHoles() const
{
    return m_Holes;
}

void PolygonNode::setHoles(const VectorVec2Vector& holes)
{
    m_Holes = holes;
    m_TexCoords.clear();
    m_EffTexCoords.clear();
    setDrawNeeded();
}

void PolygonNode::setTexCoords(const vector<float>& coords)
{
    if (coords.size() != m_Pts.size()+1 && coords.size() != 2 && coords.size() != 0) {
        throw(Exception(AVG_ERR_OUT_OF_RANGE, 
                "Illegal number of texture coordinates in polygon. Number of elements must be 0, 2 or the number of vertexes."));
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

void PolygonNode::getElementsByPos(const glm::vec2& pos, vector<NodePtr>& pElements)
{
    if (reactsToMouseEvents() && pointInPolygon(pos, m_Pts)) {
        pElements.push_back(getSharedThis());
    }
}

void PolygonNode::calcVertexes(const VertexDataPtr& pVertexData, Pixel32 color)
{
    if (getNumDifferentPts(m_Pts) < 3) {
        return;
    }
    if (m_EffTexCoords.empty()) {
        calcEffPolyLineTexCoords(m_EffTexCoords, m_TexCoords, m_CumulDist);
    }
    calcPolyLine(m_Pts, m_EffTexCoords, true, m_LineJoin, pVertexData, color);

    for (unsigned i = 0; i < m_Holes.size(); i++) {
        calcPolyLine(m_Holes[i], m_EffTexCoords, true, m_LineJoin, pVertexData, color);
    }
}

void PolygonNode::calcFillVertexes(const VertexDataPtr& pVertexData, Pixel32 color)
{
    if (getNumDifferentPts(m_Pts) < 3) {
        return;
    }
    // Remove duplicate points
    vector<glm::vec2> pts;
    vector<unsigned int> holeIndexes;
    pts.reserve(m_Pts.size());

    if (glm::distance2(m_Pts[0], m_Pts[m_Pts.size()-1]) > 0.1) {
        pts.push_back(m_Pts[0]);
    }
    for (unsigned i = 1; i < m_Pts.size(); ++i) {
        if (glm::distance2(m_Pts[i], m_Pts[i-1]) > 0.1) {
            pts.push_back(m_Pts[i]);
        }
    }

    if (m_Holes.size() > 0) {
        for (unsigned int i = 0; i < m_Holes.size(); i++) { //loop over collection
            holeIndexes.push_back(pts.size());
            for (unsigned int j = 0; j < m_Holes[i].size(); j++) { //loop over vector
                pts.push_back(m_Holes[i][j]);
            }
        }
    }
    if (color.getA() > 0) {
        glm::vec2 minCoord = pts[0];
        glm::vec2 maxCoord = pts[0];
        for (unsigned i = 1; i < pts.size(); ++i) {
            if (pts[i].x < minCoord.x) {
                minCoord.x = pts[i].x;
            }
            if (pts[i].x > maxCoord.x) {
                maxCoord.x = pts[i].x;
            }
            if (pts[i].y < minCoord.y) {
                minCoord.y = pts[i].y;
            }
            if (pts[i].y > maxCoord.y) {
                maxCoord.y = pts[i].y;
            }
        }
        vector<unsigned int> triIndexes;
        triangulatePolygon(triIndexes, pts, holeIndexes);

        for (unsigned i = 0; i < pts.size(); ++i) {
            glm::vec2 texCoord = calcFillTexCoord(pts[i], minCoord, maxCoord);
            pVertexData->appendPos(pts[i], texCoord, color);
        }
        for (unsigned i = 0; i < triIndexes.size(); i+=3) {
            pVertexData->appendTriIndexes(triIndexes[i], triIndexes[i+1], 
                    triIndexes[i+2]);
        }
    }
}

}
