//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2020 Ulrich von Zadow
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
#include "TypeRegistry.h"

#include "../base/Exception.h"
#include "../base/GeomHelper.h"
#include "../base/Polygon.h"
#include "../graphics/VertexData.h"

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
        ;
    TypeRegistry::get()->registerType(def);
}

PolygonNode::PolygonNode(const ArgList& args, const string& sPublisherName)
    : FilledVectorNode(args, sPublisherName),
      m_bPtsChanged(true)
{
    args.setMembers(this);
    if (m_TexCoords.size() > m_Pts.size()+1) {
        throw(Exception(AVG_ERR_OUT_OF_RANGE, 
                "Too many texture coordinates in polygon"));
    }
    setLineJoin(args.getArgVal<string>("linejoin"));
    calcPolyLineCumulDist(m_CumulDist, m_Pts, true);
    m_bPtsChanged = true;
    triangulate();
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
    m_bPtsChanged = true;
}
        
const vector<float>& PolygonNode::getTexCoords() const
{
    return m_TexCoords;
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

void PolygonNode::calcVertexes(const VertexDataPtr& pVertexData, Pixel32 color)
{
    if (getNumDifferentPts(m_Pts) < 3) {
        return;
    }
    if (m_EffTexCoords.empty()) {
        calcEffPolyLineTexCoords(m_EffTexCoords, m_TexCoords, m_CumulDist);
    }
    calcPolyLine(m_Pts, m_EffTexCoords, true, m_LineJoin, pVertexData, color);
}

void PolygonNode::calcFillVertexes(const VertexDataPtr& pVertexData, Pixel32 color)
{
    if (isFillVisible()) {
        triangulate();
        if (m_TriIndexes.empty()) {
            return;
        }

        glm::vec2 minCoord = m_TriPts[0];
        glm::vec2 maxCoord = m_TriPts[0];
        for (unsigned i = 1; i < m_TriPts.size(); ++i) {
            if (m_TriPts[i].x < minCoord.x) {
                minCoord.x = m_TriPts[i].x;
            }
            if (m_TriPts[i].x > maxCoord.x) {
                maxCoord.x = m_TriPts[i].x;
            }
            if (m_TriPts[i].y < minCoord.y) {
                minCoord.y = m_TriPts[i].y;
            }
            if (m_TriPts[i].y > maxCoord.y) {
                maxCoord.y = m_TriPts[i].y;
            }
        }

        for (unsigned i = 0; i < m_TriPts.size(); ++i) {
            glm::vec2 texCoord = calcFillTexCoord(m_TriPts[i], minCoord, maxCoord);
            pVertexData->appendPos(m_TriPts[i], texCoord, color);
        }
        for (unsigned i = 0; i < m_TriIndexes.size(); i+=3) {
            pVertexData->appendTriIndexes(m_TriIndexes[i], m_TriIndexes[i+1], 
                    m_TriIndexes[i+2]);
        }
    }
}

bool PolygonNode::isInside(const glm::vec2& pos)
{
    return (FilledVectorNode::isInside(pos) || pointInPolygon(pos, m_Pts));
}


void PolygonNode::triangulate()
{
    if (m_bPtsChanged) {
        m_TriPts.clear();
        m_TriIndexes.clear();
        if (getNumDifferentPts(m_Pts) < 3) {
            return;
        }
        Polygon poly(m_Pts);
        poly.triangulate(m_TriPts, m_TriIndexes);
        m_bPtsChanged = false;
    }
}

}
