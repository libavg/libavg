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

#include "CircleNode.h"

#include "TypeDefinition.h"

#include "../base/Exception.h"
#include "../base/MathHelper.h"

#include <iostream>
#include <sstream>

using namespace std;

namespace avg {

void CircleNode::registerType()
{
    TypeDefinition def = TypeDefinition("circle", "filledvectornode",
            ExportedObject::buildObject<CircleNode>)
        .addArg(Arg<glm::vec2>("pos", glm::vec2(0,0), false, offsetof(CircleNode, m_Pos)))
        .addArg(Arg<float>("r", 1, false, offsetof(CircleNode, m_Radius)))
        .addArg(Arg<float>("texcoord1", 0, false, offsetof(CircleNode, m_TC1)))
        .addArg(Arg<float>("texcoord2", 1, false, offsetof(CircleNode, m_TC2)))
        ;
    TypeRegistry::get()->registerType(def);
}

CircleNode::CircleNode(const ArgList& args)
    : FilledVectorNode(args)
{
    args.setMembers(this);
}

CircleNode::~CircleNode()
{
}

const glm::vec2& CircleNode::getPos() const 
{
    return m_Pos;
}

void CircleNode::setPos(const glm::vec2& pt) 
{
    m_Pos = pt;
    setDrawNeeded();
}

float CircleNode::getR() const 
{
    return m_Radius;
}

void CircleNode::setR(float r) 
{
    if (int(r) <= 0) {
        throw Exception(AVG_ERR_OUT_OF_RANGE, "Circle radius must be a positive number.");
    }
    m_Radius = r;
    setDrawNeeded();
}

float CircleNode::getTexCoord1() const
{
    return m_TC1;
}

void CircleNode::setTexCoord1(float tc)
{
    m_TC1 = tc;
    setDrawNeeded();
}

float CircleNode::getTexCoord2() const
{
    return m_TC2;
}

void CircleNode::setTexCoord2(float tc)
{
    m_TC2 = tc;
    setDrawNeeded();
}

void CircleNode::getElementsByPos(const glm::vec2& pos, vector<NodePtr>& pElements)
{
    if (glm::length(pos-m_Pos) <= m_Radius && reactsToMouseEvents()) {
        pElements.push_back(getSharedThis());
    }
}

void CircleNode::calcVertexes(const VertexDataPtr& pVertexData, Pixel32 color)
{
    // TODO: This gets called whenever the circle position changes and is quite 
    // expensive. Should be optimized away.
    glm::vec2 firstPt1 = getCirclePt(0, m_Radius+getStrokeWidth()/2)+m_Pos;
    glm::vec2 firstPt2 = getCirclePt(0, m_Radius-getStrokeWidth()/2)+m_Pos;
    int curVertex = 0;
    pVertexData->appendPos(firstPt1, glm::vec2(m_TC1, 0), color);
    pVertexData->appendPos(firstPt2, glm::vec2(m_TC1, 1), color);
    vector<glm::vec2> innerCircle;
    getEigthCirclePoints(innerCircle, m_Radius-getStrokeWidth()/2);
    vector<glm::vec2> outerCircle;
    getEigthCirclePoints(outerCircle, m_Radius+getStrokeWidth()/2);
    
    typedef vector<glm::vec2>::iterator Vec2It;
    typedef vector<glm::vec2>::reverse_iterator Vec2RIt;
    int i = 0;
    for (Vec2It iit = innerCircle.begin()+1, oit = outerCircle.begin()+1; 
            iit != innerCircle.end(); ++iit, ++oit)
    {
        appendCirclePoint(pVertexData, *iit, *oit, color, i, curVertex);
    }
    for (Vec2RIt iit = innerCircle.rbegin()+1, oit = outerCircle.rbegin()+1; 
            iit != innerCircle.rend(); ++iit, ++oit)
    {
        glm::vec2 iPt = glm::vec2(-iit->y, -iit->x);
        glm::vec2 oPt = glm::vec2(-oit->y, -oit->x);
        appendCirclePoint(pVertexData, iPt, oPt, color, i, curVertex);
    }
    for (Vec2It iit = innerCircle.begin()+1, oit = outerCircle.begin()+1; 
            iit != innerCircle.end(); ++iit, ++oit)
    {
        glm::vec2 iPt = glm::vec2(-iit->y, iit->x);
        glm::vec2 oPt = glm::vec2(-oit->y, oit->x);
        appendCirclePoint(pVertexData, iPt, oPt, color, i, curVertex);
    }
    for (Vec2RIt iit = innerCircle.rbegin()+1, oit = outerCircle.rbegin()+1; 
            iit !=innerCircle.rend(); ++iit, ++oit)
    {
        glm::vec2 iPt = glm::vec2(iit->x, -iit->y);
        glm::vec2 oPt = glm::vec2(oit->x, -oit->y);
        appendCirclePoint(pVertexData, iPt, oPt, color, i, curVertex);
    }
    for (Vec2It iit = innerCircle.begin()+1, oit = outerCircle.begin()+1; 
            iit != innerCircle.end(); ++iit, ++oit)
    {
        glm::vec2 iPt = glm::vec2(-iit->x, -iit->y);
        glm::vec2 oPt = glm::vec2(-oit->x, -oit->y);
        appendCirclePoint(pVertexData, iPt, oPt, color, i, curVertex);
    }
    for (Vec2RIt iit = innerCircle.rbegin()+1, oit = outerCircle.rbegin()+1; 
            iit != innerCircle.rend(); ++iit, ++oit)
    {
        glm::vec2 iPt = glm::vec2(iit->y, iit->x);
        glm::vec2 oPt = glm::vec2(oit->y, oit->x);
        appendCirclePoint(pVertexData, iPt, oPt, color, i, curVertex);
    }
    for (Vec2It iit = innerCircle.begin()+1, oit = outerCircle.begin()+1; 
            iit != innerCircle.end(); ++iit, ++oit)
    {
        glm::vec2 iPt = glm::vec2(iit->y, -iit->x);
        glm::vec2 oPt = glm::vec2(oit->y, -oit->x);
        appendCirclePoint(pVertexData, iPt, oPt, color, i, curVertex);
    }
    for (Vec2RIt iit = innerCircle.rbegin()+1, oit = outerCircle.rbegin()+1; 
            iit != innerCircle.rend(); ++iit, ++oit)
    {
        glm::vec2 iPt = glm::vec2(-iit->x, iit->y);
        glm::vec2 oPt = glm::vec2(-oit->x, oit->y);
        appendCirclePoint(pVertexData, iPt, oPt, color, i, curVertex);
    }
}

void CircleNode::calcFillVertexes(const VertexDataPtr& pVertexData, Pixel32 color)
{
    glm::vec2 minPt = m_Pos-glm::vec2(m_Radius, m_Radius);
    glm::vec2 maxPt = m_Pos+glm::vec2(m_Radius, m_Radius);
    glm::vec2 centerTexCoord = calcFillTexCoord(m_Pos, minPt, maxPt);
    pVertexData->appendPos(m_Pos, centerTexCoord, color);
    int curVertex = 1;
    glm::vec2 firstPt = getCirclePt(0, m_Radius)+m_Pos;
    glm::vec2 firstTexCoord = calcFillTexCoord(firstPt, minPt, maxPt);
    pVertexData->appendPos(firstPt, firstTexCoord, color);
    vector<glm::vec2> circlePoints;
    getEigthCirclePoints(circlePoints, m_Radius);

    for (vector<glm::vec2>::iterator it = circlePoints.begin()+1;
            it != circlePoints.end(); ++it)
    {
        glm::vec2 curPt = *it+m_Pos;
        appendFillCirclePoint(pVertexData, curPt, minPt, maxPt, color, curVertex);
    }
    for (vector<glm::vec2>::reverse_iterator it = circlePoints.rbegin()+1; 
            it != circlePoints.rend(); ++it)
    {
        glm::vec2 curPt = glm::vec2(-it->y, -it->x)+m_Pos;
        appendFillCirclePoint(pVertexData, curPt, minPt, maxPt, color, curVertex);
    }
    for (vector<glm::vec2>::iterator it = circlePoints.begin()+1;
            it != circlePoints.end(); ++it)
    {
        glm::vec2 curPt = glm::vec2(-it->y, it->x)+m_Pos;
        appendFillCirclePoint(pVertexData, curPt, minPt, maxPt, color, curVertex);
    }
    for (vector<glm::vec2>::reverse_iterator it = circlePoints.rbegin()+1; 
            it != circlePoints.rend(); ++it)
    {
        glm::vec2 curPt = glm::vec2(it->x, -it->y)+m_Pos;
        appendFillCirclePoint(pVertexData, curPt, minPt, maxPt, color, curVertex);
    }
    for (vector<glm::vec2>::iterator it = circlePoints.begin()+1;
            it != circlePoints.end(); ++it)
    {
        glm::vec2 curPt = glm::vec2(-it->x, -it->y)+m_Pos;
        appendFillCirclePoint(pVertexData, curPt, minPt, maxPt, color, curVertex);
    }
    for (vector<glm::vec2>::reverse_iterator it = circlePoints.rbegin()+1;
            it != circlePoints.rend(); ++it)
    {
        glm::vec2 curPt = glm::vec2(it->y, it->x)+m_Pos;
        appendFillCirclePoint(pVertexData, curPt, minPt, maxPt, color, curVertex);
    }
    for (vector<glm::vec2>::iterator it = circlePoints.begin()+1;
            it != circlePoints.end(); ++it)
    {
        glm::vec2 curPt = glm::vec2(it->y, -it->x)+m_Pos;
        appendFillCirclePoint(pVertexData, curPt, minPt, maxPt, color, curVertex);
    }
    for (vector<glm::vec2>::reverse_iterator it = circlePoints.rbegin()+1;
            it != circlePoints.rend(); ++it)
    {
        glm::vec2 curPt = glm::vec2(-it->x, it->y)+m_Pos;
        appendFillCirclePoint(pVertexData, curPt, minPt, maxPt, color, curVertex);
    }
}

void CircleNode::appendCirclePoint(const VertexDataPtr& pVertexData, 
        const glm::vec2& iPt, const glm::vec2& oPt, Pixel32 color, int& i, 
        int& curVertex)
{
    i++;
    float ratio = (float(i)/getNumCircumferencePoints());
    float curTC = (1-ratio)*m_TC1+ratio*m_TC2;
    pVertexData->appendPos(oPt+m_Pos, glm::vec2(curTC, 0), color);
    pVertexData->appendPos(iPt+m_Pos, glm::vec2(curTC, 1), color);
    pVertexData->appendQuadIndexes(curVertex+1, curVertex, curVertex+3, curVertex+2); 
    curVertex += 2;
}

void CircleNode::appendFillCirclePoint(const VertexDataPtr& pVertexData, 
        const glm::vec2& curPt, const glm::vec2& minPt, const glm::vec2& maxPt,
        Pixel32 color, int& curVertex)
{
    glm::vec2 curTexCoord = calcFillTexCoord(curPt, minPt, maxPt);
    pVertexData->appendPos(curPt, curTexCoord, color);
    pVertexData->appendTriIndexes(0, curVertex, curVertex+1);
    curVertex++;
}

int CircleNode::getNumCircumferencePoints()
{
    return int(ceil((m_Radius*3)/8)*8);
}

void CircleNode::getEigthCirclePoints(vector<glm::vec2>& pts, float radius)
{
    int numPts = getNumCircumferencePoints();
    for (int i = 0; i <= numPts/8; ++i) {
        float ratio = (float(i)/numPts);
        float angle = ratio*2*PI;
        pts.push_back(getCirclePt(angle, radius));
    }
}

glm::vec2 CircleNode::getCirclePt(float angle, float radius)
{
    return glm::vec2(sin(angle)*radius, -cos(angle)*radius);
}

}
