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

#include "LineNode.h"

#include "TypeDefinition.h"

#include "../base/Exception.h"

#include <iostream>
#include <sstream>

using namespace std;

namespace avg {

void LineNode::registerType()
{
    TypeDefinition def = TypeDefinition("line", "vectornode", 
            ExportedObject::buildObject<LineNode>)
        .addArg(Arg<glm::vec2>("pos1", glm::vec2(0,0), false, offsetof(LineNode, m_P1)))
        .addArg(Arg<glm::vec2>("pos2", glm::vec2(0,0), false, offsetof(LineNode, m_P2)))
        .addArg(Arg<float>("texcoord1", 0, false, offsetof(LineNode, m_TC1)))
        .addArg(Arg<float>("texcoord2", 1, false, offsetof(LineNode, m_TC2)))
        ;
    TypeRegistry::get()->registerType(def);
}

LineNode::LineNode(const ArgList& args)
    : VectorNode(args)
{
    args.setMembers(this);
}

LineNode::~LineNode()
{
}

const glm::vec2& LineNode::getPos1() const 
{
    return m_P1;
}

void LineNode::setPos1(const glm::vec2& pt) 
{
    m_P1 = pt;
    setDrawNeeded();
}

const glm::vec2& LineNode::getPos2() const 
{
    return m_P2;
}

void LineNode::setPos2(const glm::vec2& pt) 
{
    m_P2 = pt;
    setDrawNeeded();
}

float LineNode::getTexCoord1() const
{
    return m_TC1;
}

void LineNode::setTexCoord1(float tc)
{
    m_TC1 = tc;
    setDrawNeeded();
}

float LineNode::getTexCoord2() const
{
    return m_TC2;
}

void LineNode::setTexCoord2(float tc)
{
    m_TC2 = tc;
    setDrawNeeded();
}

void LineNode::calcVertexes(const VertexDataPtr& pVertexData, Pixel32 color)
{
    pVertexData->addLineData(color, m_P1, m_P2, getStrokeWidth(), m_TC1, m_TC2);
}

}
