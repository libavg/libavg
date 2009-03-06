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
    m_TexCoords.clear();
    m_EffTexCoords.clear();
    calcPolyLineCumulDist(m_CumulDist, m_Pts);
    setDrawNeeded(true);
}
        
const vector<double>& PolyLineNode::getTexCoords() const
{
    return m_TexCoords;
}

void PolyLineNode::setTexCoords(const vector<double>& coords)
{
    if (coords.size() > m_Pts.size()) {
        throw(Exception(AVG_ERR_OUT_OF_RANGE, 
                "Number of texture coordinates in polyline or polygon must match number of points."));
    }
    m_EffTexCoords.clear();
    m_TexCoords = coords;
    setDrawNeeded(false);
}

string PolyLineNode::getLineJoin() const
{
    return lineJoin2String(m_LineJoin);
}

void PolyLineNode::setLineJoin(const string& s)
{
    m_LineJoin = string2LineJoin(s);
    setDrawNeeded(true);
}

int PolyLineNode::getNumVertexes()
{
    if (m_Pts.size() < 2) {
        return 0;
    }
    int numPts = m_Pts.size();
    for (unsigned i=1; i<m_Pts.size(); ++i) {
        if (calcDistSquared(m_Pts[i], m_Pts[i-1])<0.1) {
            numPts--;
        }
    }
    switch (m_LineJoin) {
        case LJ_MITER:
            return 2*numPts;
        case LJ_BEVEL:
            return 3*numPts-2;
        default:
            assert(false);
            return 0;
    }

}

int PolyLineNode::getNumIndexes()
{
    if (m_Pts.size() < 2) {
        return 0;
    }
    int numPts = m_Pts.size();
    for (unsigned i=1; i<m_Pts.size(); ++i) {
        if (calcDistSquared(m_Pts[i], m_Pts[i-1])<0.1) {
            numPts--;
        }
    }
    switch (m_LineJoin) {
        case LJ_MITER:
            return 6*(numPts-1);
        case LJ_BEVEL:
            return 3*(3*numPts-4);
        default:
            assert(false);
            return 0;
    }
}

void PolyLineNode::calcVertexes(VertexArrayPtr& pVertexArray, Pixel32 color)
{
    if (m_Pts.size() < 2) {
        return;
    }
    if (m_EffTexCoords.empty()) {
        calcEffTexCoords();
    }
    calcPolyLine(m_Pts, m_EffTexCoords, false, m_LineJoin, pVertexArray, color);
}

void PolyLineNode::calcEffTexCoords()
{
    if (m_TexCoords.empty()) {
        m_EffTexCoords = m_CumulDist;
    } else if (m_TexCoords.size() == m_CumulDist.size()) {
        m_EffTexCoords = m_TexCoords;
    } else {
        m_EffTexCoords.reserve(m_CumulDist.size());
        m_EffTexCoords = m_TexCoords;
        double minGivenTexCoord = m_TexCoords[0];
        double maxGivenTexCoord = m_TexCoords[m_TexCoords.size()-1];
        double maxCumulDist = m_CumulDist[m_TexCoords.size()-1];
        int baselineDist = 0;
        for (unsigned i=m_TexCoords.size(); i<m_CumulDist.size(); ++i) {
            int repeatFactor = int(m_CumulDist[i]/maxCumulDist);
            double effCumulDist = fmod(m_CumulDist[i], maxCumulDist);
            while (m_CumulDist[baselineDist+1] < effCumulDist) {
                baselineDist++;
            }
            double ratio = (effCumulDist-m_CumulDist[baselineDist])/
                    (m_CumulDist[baselineDist+1]-m_CumulDist[baselineDist]);
            double rawTexCoord = (1-ratio)*m_TexCoords[baselineDist]
                    +ratio*m_TexCoords[baselineDist+1];
            double texCoord = rawTexCoord
                    +repeatFactor*(maxGivenTexCoord-minGivenTexCoord);
            m_EffTexCoords.push_back(texCoord);
        }
    }
}

}
