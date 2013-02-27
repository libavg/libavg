//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2011 Ulrich von Zadow
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

#include "CurveNode.h"

#include "TypeDefinition.h"

#include "../graphics/VertexArray.h"
#include "../base/Exception.h"
#include "../base/MathHelper.h"
#include "../base/BezierCurve.h"

#include <math.h>
#include <float.h>
#include <iostream>
#include <sstream>

using namespace std;

namespace avg {

void CurveNode::registerType()
{
    TypeDefinition def = TypeDefinition("curve", "vectornode", 
            ExportedObject::buildObject<CurveNode>)
        .addArg(Arg<glm::vec2>("pos1", glm::vec2(0,0), false, offsetof(CurveNode, m_P1)))
        .addArg(Arg<glm::vec2>("pos2", glm::vec2(0,0), false, offsetof(CurveNode, m_P2)))
        .addArg(Arg<glm::vec2>("pos3", glm::vec2(0,0), false, offsetof(CurveNode, m_P3)))
        .addArg(Arg<glm::vec2>("pos4", glm::vec2(0,0), false, offsetof(CurveNode, m_P4)))
        .addArg(Arg<float>("texcoord1", 0, true, offsetof(CurveNode, m_TC1)))
        .addArg(Arg<float>("texcoord2", 1, true, offsetof(CurveNode, m_TC2)));
    TypeRegistry::get()->registerType(def);
}

CurveNode::CurveNode(const ArgList& args)
   : VectorNode(args)
{
    args.setMembers(this);
}

CurveNode::~CurveNode()
{
}

const glm::vec2& CurveNode::getPos1() const 
{
    return m_P1;
}

void CurveNode::setPos1(const glm::vec2& pt) 
{
    m_P1 = pt;
    setDrawNeeded();
}

const glm::vec2& CurveNode::getPos2() const 
{
    return m_P2;
}

void CurveNode::setPos2(const glm::vec2& pt) 
{
    m_P2 = pt;
    setDrawNeeded();
}

const glm::vec2& CurveNode::getPos3() const 
{
    return m_P3;
}

void CurveNode::setPos3(const glm::vec2& pt) 
{
    m_P3 = pt;
    setDrawNeeded();
}

const glm::vec2& CurveNode::getPos4() const 
{
    return m_P4;
}

void CurveNode::setPos4(const glm::vec2& pt) 
{
    m_P4 = pt;
    setDrawNeeded();
}

float CurveNode::getTexCoord1() const
{
    return m_TC1;
}

void CurveNode::setTexCoord1(float tc)
{
    m_TC1 = tc;
    setDrawNeeded();
}

float CurveNode::getTexCoord2() const
{
    return m_TC2;
}

void CurveNode::setTexCoord2(float tc)
{
    m_TC2 = tc;
    setDrawNeeded();
}

void CurveNode::calcVertexes(const VertexDataPtr& pVertexData, Pixel32 color)
{
    updateLines();
    
    pVertexData->appendPos(m_LeftCurve[0], glm::vec2(m_TC1,1), color);
    pVertexData->appendPos(m_RightCurve[0], glm::vec2(m_TC2,0), color);
    for (unsigned i = 0; i < m_LeftCurve.size()-1; ++i) {
        float ratio = i/float(m_LeftCurve.size());
        float tc = (1-ratio)*m_TC1+ratio*m_TC2;
        pVertexData->appendPos(m_LeftCurve[i+1], glm::vec2(tc,1), color);
        pVertexData->appendPos(m_RightCurve[i+1], glm::vec2(tc,0), color);
        pVertexData->appendQuadIndexes((i+1)*2, i*2, (i+1)*2+1, i*2+1);
    }
}

int CurveNode::getCurveLen()
{
    // Calc. upper bound for spline length.
    float curveLen = glm::length(m_P2-m_P1) + glm::length(m_P3 - m_P2)
            + glm::length(m_P4-m_P3);
    if (curveLen > 50000) {
        throw Exception(AVG_ERR_OUT_OF_RANGE, "Illegal points in curve.");
    }
    return int(curveLen);
}

void CurveNode::updateLines()
{
    BezierCurve curve(m_P1, m_P2, m_P3, m_P4);
    
    float len = float(getCurveLen());
    m_LeftCurve.clear();
    m_RightCurve.clear();
    m_LeftCurve.reserve(int(len+1.5f));
    m_RightCurve.reserve(int(len+1.5f));

    for (unsigned i = 0; i < len; ++i) {
        float t = i/len;
        addLRCurvePoint(curve.interpolate(t), curve.getDeriv(t));
    }
    addLRCurvePoint(curve.interpolate(1), curve.getDeriv(1));
}

void CurveNode::addLRCurvePoint(const glm::vec2& pos, const glm::vec2& deriv)
{
    glm::vec2 m = glm::normalize(deriv);
    glm::vec2 w = glm::vec2(m.y, -m.x)*float(getStrokeWidth()/2);
    m_LeftCurve.push_back(pos-w);
    m_RightCurve.push_back(pos+w);
}

}
