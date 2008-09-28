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

#include "VectorNode.h"

#include "NodeDefinition.h"

#include "../graphics/VertexArray.h"
#include "../base/Exception.h"

#include <iostream>
#include <sstream>

using namespace std;

namespace avg {

NodeDefinition VectorNode::createDefinition()
{
    return NodeDefinition("vector")
        .extendDefinition(Node::createDefinition())
        .addArg(Arg<string>("color", "FFFFFF", false, offsetof(VectorNode, m_sColorName)))
        .addArg(Arg<double>("strokewidth", 1, false, offsetof(VectorNode, m_StrokeWidth)));
}

VectorNode::VectorNode(const ArgList& Args)
{
}

VectorNode::~VectorNode()
{
}

void VectorNode::setRenderingEngines(DisplayEngine * pDisplayEngine, 
        AudioEngine * pAudioEngine)
{
    m_bDrawNeeded = true;
    m_Color = colorStringToColor(m_sColorName);
    Node::setRenderingEngines(pDisplayEngine, pAudioEngine);
}

void VectorNode::setColor(const string& sColor)
{
    if (m_sColorName != sColor) {
        m_sColorName = sColor;
        m_Color = colorStringToColor(m_sColorName);
        m_bDrawNeeded = true;
    }
}

const string& VectorNode::getColor() const
{
    return m_sColorName;
}

void VectorNode::setStrokeWidth(double width)
{
    if (width != m_StrokeWidth) {
        m_bDrawNeeded = true;
        m_StrokeWidth = width;
    }
}

double VectorNode::getStrokeWidth() const
{
    return m_StrokeWidth;
}

Pixel32 VectorNode::getColorVal() const
{
    return m_Color;
}

void VectorNode::updateLineData(VertexArrayPtr pVertexArray, int triIndex, double opacity,
        const DPoint& p1, const DPoint& p2)
{
    double curOpacity = opacity*getOpacity();
    Pixel32 color = getColorVal();
    color.setA(curOpacity*255);

    DPoint m = (p2-p1);
    m.normalize();
    DPoint w = DPoint(m.y, -m.x)*getStrokeWidth()/2;
    pVertexArray->setPos(triIndex, 0, p1-w, DPoint(0,0), color);
    pVertexArray->setPos(triIndex, 1, p1+w, DPoint(0,0), color);
    pVertexArray->setPos(triIndex, 2, p2+w, DPoint(0,0), color);

    pVertexArray->setPos(triIndex+1, 0, p1-w, DPoint(0,0), color);
    pVertexArray->setPos(triIndex+1, 1, p2+w, DPoint(0,0), color);
    pVertexArray->setPos(triIndex+1, 2, p2-w, DPoint(0,0), color);
}
     
bool VectorNode::isDrawNeeded()
{
    return m_bDrawNeeded;
}

void VectorNode::setDrawNeeded(bool bSet)
{
    m_bDrawNeeded = bSet;
}

}
