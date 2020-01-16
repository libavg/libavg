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

#include "CurveNode.h"

#include "TypeDefinition.h"
#include "TypeRegistry.h"

#include "../base/Exception.h"
#include "../base/MathHelper.h"

#include "../graphics/VertexData.h"

#include <math.h>
#include <float.h>
#include <iostream>
#include <sstream>

using namespace std;

namespace avg {

CurveAABB::CurveAABB(const glm::vec2& pt, int startIDX, int endIDX)
    : FRect(pt, pt),
      m_StartIdx(startIDX),
      m_EndIdx(endIDX)
{
}

void CurveNode::registerType()
{
    TypeDefinition def = TypeDefinition("curve", "vectornode", 
            ExportedObject::buildObject<CurveNode>)
        .addArg(Arg<glm::vec2>("pos1", glm::vec2(0,0)))
        .addArg(Arg<glm::vec2>("pos2", glm::vec2(0,0)))
        .addArg(Arg<glm::vec2>("pos3", glm::vec2(0,0)))
        .addArg(Arg<glm::vec2>("pos4", glm::vec2(0,0)))
        .addArg(Arg<float>("texcoord1", 0, true, offsetof(CurveNode, m_TC1)))
        .addArg(Arg<float>("texcoord2", 1, true, offsetof(CurveNode, m_TC2)));
    TypeRegistry::get()->registerType(def);
}

CurveNode::CurveNode(const ArgList& args, const string& sPublisherName)
   : VectorNode(args, sPublisherName)
{
    args.setMembers(this);
    glm::vec2 p0 = args.getArgVal<glm::vec2>("pos1");
    glm::vec2 p1 = args.getArgVal<glm::vec2>("pos2");
    glm::vec2 p2 = args.getArgVal<glm::vec2>("pos3");
    glm::vec2 p3 = args.getArgVal<glm::vec2>("pos4");
    m_pCurve = BezierCurvePtr(new BezierCurve(p0, p1, p2, p3));
}

CurveNode::~CurveNode()
{
}

const glm::vec2& CurveNode::getPos1() const 
{
    return m_pCurve->getPt(0);
}

void CurveNode::setPos1(const glm::vec2& pt) 
{
    m_pCurve->setPt(0, pt);
    setDrawNeeded();
}

const glm::vec2& CurveNode::getPos2() const 
{
    return m_pCurve->getPt(1);
}

void CurveNode::setPos2(const glm::vec2& pt) 
{
    m_pCurve->setPt(1, pt);
    setDrawNeeded();
}

const glm::vec2& CurveNode::getPos3() const 
{
    return m_pCurve->getPt(2);
}

void CurveNode::setPos3(const glm::vec2& pt) 
{
    m_pCurve->setPt(2, pt);
    setDrawNeeded();
}

const glm::vec2& CurveNode::getPos4() const 
{
    return m_pCurve->getPt(3);
}

void CurveNode::setPos4(const glm::vec2& pt) 
{
    m_pCurve->setPt(3, pt);
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
 
float CurveNode::getCurveLen() const
{
    return m_pCurve->estimateLen();
}

glm::vec2 CurveNode::getPtOnCurve(float t) const
{
    return m_pCurve->interpolate(t);
}

void CurveNode::calcVertexes(const VertexDataPtr& pVertexData, Pixel32 color)
{
    updateLines();
    calcBoundingBoxes();

    pVertexData->appendPos(m_LeftCurve[0], glm::vec2(m_TC1,1), color);
    pVertexData->appendPos(m_RightCurve[0], glm::vec2(m_TC2,0), color);
    for (unsigned i = 0; i < m_LeftCurve.size()-1; ++i) {
        float ratio = (i+1)/(float(m_LeftCurve.size()));
        float tc = (1-ratio)*m_TC1+ratio*m_TC2;
        pVertexData->appendPos(m_LeftCurve[i+1], glm::vec2(tc,1), color);
        pVertexData->appendPos(m_RightCurve[i+1], glm::vec2(tc,0), color);
        pVertexData->appendQuadIndexes((i+1)*2, i*2, (i+1)*2+1, i*2+1);
    }
}

bool CurveNode::isInside(const glm::vec2& pos)
{
    glm::vec2 globalPos = toGlobal(pos);
    return isInsideBB(globalPos, m_AABBs.size()-1, 0);
}

bool CurveNode::isInsideBB(const glm::vec2& pos, unsigned level, unsigned i)
{
    if ((*m_AABBs[level])[i].contains(pos)) {
        if (level == 0) {
            // Check individual points
            const CurveAABB& aabb = (*m_AABBs[level])[i];
            for (int i=aabb.m_StartIdx; i<=aabb.m_EndIdx; ++i) {
                if (glm::distance(m_CenterCurve[i], pos) < getStrokeWidth()/2) {
                    return true;
                }
            }
            return false;
        } else {
            // Recurse to smaller bounding boxes.
            if (i*2+1 < m_AABBs[level-1]->size()) {
                return isInsideBB(pos, level-1, i*2) || isInsideBB(pos, level-1, i*2+1);
            } else {
                return isInsideBB(pos, level-1, i*2);
            }
        }
    } else {
        return false;
    }
}

void CurveNode::updateLines()
{
    unsigned numPts = unsigned(getCurveLen()/2);
    m_CenterCurve.clear();
    m_LeftCurve.clear();
    m_RightCurve.clear();
    m_CenterCurve.reserve(numPts + 2);
    m_LeftCurve.reserve(numPts + 2);
    m_RightCurve.reserve(numPts + 2);

    for (unsigned i = 0; i < numPts; ++i) {
        float t = float(i)/numPts;
        addCurvePoints(m_pCurve->interpolate(t), m_pCurve->getDeriv(t));
    }
    addCurvePoints(m_pCurve->interpolate(1), m_pCurve->getDeriv(1));
}

void CurveNode::calcBoundingBoxes()
{
    m_AABBs.clear();

    // Lowest level: Generate from curve points.
    m_AABBs.push_back(CurveAABBVectorPtr(new CurveAABBVector()));
    CurveAABBVectorPtr pCurAABBs = m_AABBs.back();
    pCurAABBs->reserve(m_CenterCurve.size()/8);
    glm::vec2 stroke(getStrokeWidth()/2, getStrokeWidth()/2);
    for (unsigned i=0; i<=m_CenterCurve.size()/8; ++i) {
        int startIdx = i*8;
        int endIdx = min(i*8+7, unsigned(m_CenterCurve.size()-1));
        const glm::vec2& curPt = m_CenterCurve[startIdx];
        pCurAABBs->push_back(CurveAABB(curPt, startIdx, endIdx));
        CurveAABB& curAABB = pCurAABBs->back();
        for (int j=startIdx; j<=endIdx; j+=2) {
            curAABB.expand(m_CenterCurve[j]);
        }
        curAABB.tl -= stroke;
        curAABB.br += stroke;
    }
    // Higher levels: combine AABBs of lower levels.
    unsigned numSections = pCurAABBs->size();
    unsigned level = 0;
    while (numSections > 1) {
        numSections = int(ceil(float(numSections)/2));
        m_AABBs.push_back(CurveAABBVectorPtr(new CurveAABBVector()));
        CurveAABBVectorPtr pLastAABBs = m_AABBs[level];
        pCurAABBs = m_AABBs.back();
        pCurAABBs->reserve(numSections);
        for (unsigned i=0; i<numSections; ++i) {
            pCurAABBs->push_back(CurveAABB((*pLastAABBs)[i*2]));
            CurveAABB& curAABB = pCurAABBs->back();
            if (i*2+1 < pLastAABBs->size()) {
                curAABB.expand((*pLastAABBs)[i*2+1]);
            }
        }
        level++;
    }
/*
    for (unsigned i=0; i<m_AABBs.size(); ++i) {
        cerr << "Level: " << i << " (size: " << m_AABBs[i]->size() << ")" << endl;
        for (unsigned j=0; j<m_AABBs[i]->size(); ++j) {
            cerr << (*m_AABBs[i])[j] << " ";
        }
        cerr << endl;
    }
    */
}

void CurveNode::addCurvePoints(const glm::vec2& pos, const glm::vec2& deriv)
{
    m_CenterCurve.push_back(pos);
    glm::vec2 m = glm::normalize(deriv);
    glm::vec2 w = glm::vec2(m.y, -m.x)*float(getStrokeWidth()/2);
    m_LeftCurve.push_back(pos-w);
    m_RightCurve.push_back(pos+w);
}

}
