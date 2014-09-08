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

#include "PolyLineNode.h"

#include "TypeDefinition.h"

#include "../base/Exception.h"

#include <iostream>
#include <sstream>

using namespace std;

namespace avg {

void PolyLineNode::registerType()
{
    vector<glm::vec2> v;
    vector<float> vd;
    TypeDefinition def = TypeDefinition("polyline", "vectornode", 
            ExportedObject::buildObject<PolyLineNode>)
        .addArg(Arg<string>("linejoin", "bevel"))
        .addArg(Arg<vector<glm::vec2> >("pos", v, false, offsetof(PolyLineNode, m_Pts)))
        .addArg(Arg<vector<float> >("texcoords", vd, false,
                offsetof(PolyLineNode, m_TexCoords)))
        ;
    TypeRegistry::get()->registerType(def);
}

PolyLineNode::PolyLineNode(const ArgList& args)
    : VectorNode(args)
{
    args.setMembers(this);
    if (m_TexCoords.size() > m_Pts.size()) {
        throw(Exception(AVG_ERR_OUT_OF_RANGE, 
                "Too many texture coordinates in polyline"));
    }
    setLineJoin(args.getArgVal<string>("linejoin"));
    calcPolyLineCumulDist(m_CumulDist, m_Pts, false);
}

PolyLineNode::~PolyLineNode()
{
}

const vector<glm::vec2>& PolyLineNode::getPos() const 
{
    return m_Pts;
}

void PolyLineNode::setPos(const vector<glm::vec2>& pts) 
{
    m_Pts = pts;
    m_TexCoords.clear();
    m_EffTexCoords.clear();
    calcPolyLineCumulDist(m_CumulDist, m_Pts, false);
    setDrawNeeded();
}
        
const vector<float>& PolyLineNode::getTexCoords() const
{
    return m_TexCoords;
}

void PolyLineNode::setTexCoords(const vector<float>& coords)
{
    if (coords.size() != m_Pts.size() && coords.size() != 2 && coords.size() != 0) {
        throw(Exception(AVG_ERR_OUT_OF_RANGE, 
                "Illegal number of texture coordinates in polyline. Number of elements must be 0, 2 or the number of vertexes."));
    }
    m_EffTexCoords.clear();
    m_TexCoords = coords;
    setDrawNeeded();
}

string PolyLineNode::getLineJoin() const
{
    return lineJoin2String(m_LineJoin);
}

void PolyLineNode::setLineJoin(const string& s)
{
    m_LineJoin = string2LineJoin(s);
    setDrawNeeded();
}

void PolyLineNode::calcVertexes(const VertexDataPtr& pVertexData, Pixel32 color)
{
    if (getNumDifferentPts(m_Pts) < 2) {
        return;
    }
    if (m_EffTexCoords.empty()) {
        calcEffPolyLineTexCoords(m_EffTexCoords, m_TexCoords, m_CumulDist);
    }
    calcPolyLine(m_Pts, m_EffTexCoords, false, m_LineJoin, pVertexData, color);
}

}
