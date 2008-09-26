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

#include "LineNode.h"

#include "NodeDefinition.h"

#include "../graphics/VertexArray.h"
#include "../base/Exception.h"

#include <iostream>
#include <sstream>

using namespace std;

namespace avg {

NodeDefinition LineNode::createDefinition()
{
    return NodeDefinition("line", Node::buildNode<LineNode>)
        .extendDefinition(Node::createDefinition())
        .addArg(Arg<double>("x1", 0, true, offsetof(LineNode, m_P1.x)))
        .addArg(Arg<double>("y1", 0, true, offsetof(LineNode, m_P1.y)))
        .addArg(Arg<double>("x2", 0, true, offsetof(LineNode, m_P2.x)))
        .addArg(Arg<double>("y2", 0, true, offsetof(LineNode, m_P2.y)))
        .addArg(Arg<string>("color", "FFFFFF", false, offsetof(LineNode, m_sColorName)))
        .addArg(Arg<double>("width", 1, false, offsetof(LineNode, m_Width)));
}

LineNode::LineNode (const ArgList& Args, bool bFromXML)
{
    Args.setMembers(this);
    m_Color = colorStringToColor(m_sColorName);
}

LineNode::~LineNode()
{
}

void LineNode::setRenderingEngines(DisplayEngine * pDisplayEngine, 
        AudioEngine * pAudioEngine)
{
    m_bDrawNeeded = true;
    Node::setRenderingEngines(pDisplayEngine, pAudioEngine);
}

int LineNode::getNumTriangles()
{
    return 2;
}

void LineNode::updateData(VertexArrayPtr pVertexArray, int triIndex)
{
    if (m_bDrawNeeded) {
        DPoint m = (m_P2-m_P1);
        m.normalize();
        DPoint w = DPoint(m.y, -m.x)*m_Width/2;
        pVertexArray->setPos(triIndex, 0, m_P1-w, DPoint(0,0), m_Color);
        pVertexArray->setPos(triIndex, 1, m_P1+w, DPoint(0,0), m_Color);
        pVertexArray->setPos(triIndex, 2, m_P2+w, DPoint(0,0), m_Color);

        pVertexArray->setPos(triIndex+1, 0, m_P1-w, DPoint(0,0), m_Color);
        pVertexArray->setPos(triIndex+1, 1, m_P2+w, DPoint(0,0), m_Color);
        pVertexArray->setPos(triIndex+1, 2, m_P2-w, DPoint(0,0), m_Color);
    }
    m_bDrawNeeded = false;
}

}
