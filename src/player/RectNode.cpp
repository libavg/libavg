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

#include "RectNode.h"

#include "TypeDefinition.h"

#include "../graphics/VertexArray.h"
#include "../base/Exception.h"
#include "../base/MathHelper.h"

#include <iostream>
#include <sstream>

using namespace std;

namespace avg {

void RectNode::registerType()
{
    float texCoords[] = {0, 0.25f, 0.5f, 0.75f, 1};
    TypeDefinition def = TypeDefinition("rect", "filledvectornode",
            ExportedObject::buildObject<RectNode>)
        .addArg(Arg<glm::vec2>("pos", glm::vec2(0,0), false, 
                offsetof(RectNode, m_Rect.tl)))
        .addArg(Arg<glm::vec2>("size", glm::vec2(0,0)))
        .addArg(Arg<float>("angle", 0.0f, false, offsetof(RectNode, m_Angle)))
        .addArg(Arg<vector<float> >("texcoords", vectorFromCArray(5, texCoords), false,
                offsetof(RectNode, m_TexCoords)))
        ;
    TypeRegistry::get()->registerType(def);
}

RectNode::RectNode(const ArgList& args)
    : FilledVectorNode(args)
{
    args.setMembers(this);
    setSize(args.getArgVal<glm::vec2>("size"));
}

RectNode::~RectNode()
{
}

const glm::vec2& RectNode::getPos() const 
{
    return m_Rect.tl;
}

void RectNode::setPos(const glm::vec2& pt) 
{
    float w = m_Rect.width();
    float h = m_Rect.height();
    m_Rect.tl = pt;
    m_Rect.setWidth(w);
    m_Rect.setHeight(h);
    setDrawNeeded();
}

glm::vec2 RectNode::getSize() const 
{
    return m_Rect.size();
}

void RectNode::setSize(const glm::vec2& pt) 
{
    m_Rect.setWidth(pt.x);
    m_Rect.setHeight(pt.y);
    notifySubscribers("SIZE_CHANGED", m_Rect.size());
    setDrawNeeded();
}

const vector<float>& RectNode::getTexCoords() const
{
    return m_TexCoords;
}

void RectNode::setTexCoords(const vector<float>& coords)
{
    if (coords.size() != 5) {
        throw(Exception(AVG_ERR_OUT_OF_RANGE, 
                "Number of texture coordinates for a rectangle must be 5."));
    }
    m_TexCoords = coords;
    setDrawNeeded();
}

float RectNode::getAngle() const
{
    return m_Angle;
}

void RectNode::setAngle(float angle)
{
    m_Angle = fmod(angle, 2*PI);
    setDrawNeeded();
}

void RectNode::getElementsByPos(const glm::vec2& pos, vector<NodePtr>& pElements)
{
    glm::vec2 pivot = m_Rect.tl + m_Rect.size()/2.f;
    glm::vec2 rpos = getRotatedPivot(pos, m_Angle, pivot);
    if (rpos.x >= m_Rect.tl.x && rpos.y >= m_Rect.tl.y && rpos.x < m_Rect.br.x && 
            rpos.y < m_Rect.br.y && reactsToMouseEvents())
    {
        pElements.push_back(getSharedThis());
    }
}

void RectNode::calcVertexes(const VertexDataPtr& pVertexData, Pixel32 color)
{
    glm::vec2 pivot = m_Rect.tl+m_Rect.size()/2.f;

    glm::vec2 p1 = m_Rect.tl;
    glm::vec2 p2(m_Rect.tl.x, m_Rect.br.y);
    glm::vec2 p3 = m_Rect.br;
    glm::vec2 p4(m_Rect.br.x, m_Rect.tl.y);
    
    vector<glm::vec2> pts; 
    pts.push_back(getRotatedPivot(p1, m_Angle, pivot));
    pts.push_back(getRotatedPivot(p2, m_Angle, pivot));
    pts.push_back(getRotatedPivot(p3, m_Angle, pivot));
    pts.push_back(getRotatedPivot(p4, m_Angle, pivot));
    calcPolyLine(pts, m_TexCoords, true, LJ_MITER, pVertexData, color);
}

void RectNode::calcFillVertexes(const VertexDataPtr& pVertexData, Pixel32 color)
{
    glm::vec2 pivot = m_Rect.tl+m_Rect.size()/2.f;

    glm::vec2 p1 = m_Rect.tl;
    glm::vec2 p2(m_Rect.tl.x, m_Rect.br.y);
    glm::vec2 p3 = m_Rect.br;
    glm::vec2 p4(m_Rect.br.x, m_Rect.tl.y);
    glm::vec2 rp1 = getRotatedPivot(p1, m_Angle, pivot);
    glm::vec2 rp2 = getRotatedPivot(p2, m_Angle, pivot);
    glm::vec2 rp3 = getRotatedPivot(p3, m_Angle, pivot);
    glm::vec2 rp4 = getRotatedPivot(p4, m_Angle, pivot);
    pVertexData->appendPos(rp1, getFillTexCoord1(), color);
    glm::vec2 blTexCoord = glm::vec2(getFillTexCoord1().x, getFillTexCoord2().y);
    pVertexData->appendPos(rp2, blTexCoord, color);
    pVertexData->appendPos(rp3, getFillTexCoord2(), color);
    glm::vec2 trTexCoord = glm::vec2(getFillTexCoord2().x, getFillTexCoord1().y);
    pVertexData->appendPos(rp4, trTexCoord, color);
    pVertexData->appendQuadIndexes(1, 0, 2, 3);
}

}
