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

#include <iostream>
#include <sstream>

using namespace std;

namespace avg {

NodeDefinition PolyLineNode::createDefinition()
{
    vector<DPoint> v;
    vector<double> vd;
    return NodeDefinition("polyline", VisibleNode::buildNode<PolyLineNode>)
        .extendDefinition(VectorNode::createDefinition())
        .addArg(Arg<string>("linejoin", "bevel"))
        .addArg(Arg<vector<DPoint> >("pos", v, false, offsetof(PolyLineNode, m_Pts)))
        .addArg(Arg<vector<double> >("texcoords", vd, false,
                offsetof(PolyLineNode, m_TexCoords)))
        ;
}

PolyLineNode::PolyLineNode(const ArgList& Args)
    : VectorNode(Args)
{
    Args.setMembers(this);
    if (m_TexCoords.size() > m_Pts.size()) {
        throw(Exception(AVG_ERR_OUT_OF_RANGE, 
                "Too many texture coordinates in polyline"));
    }
    setLineJoin(Args.getArgVal<string>("linejoin"));
    calcPolyLineCumulDist(m_CumulDist, m_Pts, false);
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
    vector<DPoint>::const_iterator it;
    for (it=pts.begin(); it != pts.end(); ++it) {
        if (it->isNaN() || it->isInf()) {
            throw Exception(AVG_ERR_INVALID_ARGS, 
                    "polyline positions must not be nan or inf.");
        }
    }
    m_Pts = pts;
    m_TexCoords.clear();
    m_EffTexCoords.clear();
    calcPolyLineCumulDist(m_CumulDist, m_Pts, false);
    setDrawNeeded();
}
        
const vector<double>& PolyLineNode::getTexCoords() const
{
    return m_TexCoords;
}

void PolyLineNode::setTexCoords(const vector<double>& coords)
{
    if (coords.size() > m_Pts.size()) {
        throw(Exception(AVG_ERR_OUT_OF_RANGE, 
                "Too many texture coordinates in polyline"));
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

void PolyLineNode::calcVertexes(VertexArrayPtr& pVertexArray, Pixel32 color)
{
    if (getNumDifferentPts(m_Pts) < 2) {
        return;
    }
    if (m_EffTexCoords.empty()) {
        calcEffPolyLineTexCoords(m_EffTexCoords, m_TexCoords, m_CumulDist);
    }
    calcPolyLine(m_Pts, m_EffTexCoords, false, m_LineJoin, pVertexArray, color);
}

}
