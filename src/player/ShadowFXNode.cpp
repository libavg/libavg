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

#include "ShadowFXNode.h"

#include "../base/ObjectCounter.h"
#include "../base/Exception.h"

#include <string>

using namespace std;

namespace avg {

ShadowFXNode::ShadowFXNode(glm::vec2 offset, float radius, float opacity, string sColor) 
    : FXNode(false),
      m_Offset(offset),
      m_StdDev(radius),
      m_Opacity(opacity)
{
    m_sColorName = sColor;
    m_Color = colorStringToColor(sColor);
    ObjectCounter::get()->incRef(&typeid(*this));
}

ShadowFXNode::~ShadowFXNode()
{
    ObjectCounter::get()->decRef(&typeid(*this));
}

void ShadowFXNode::connect()
{
    FXNode::connect();
}

void ShadowFXNode::disconnect()
{
    m_pFilter = GPUShadowFilterPtr();
    FXNode::disconnect();
}

void ShadowFXNode::setOffset(const glm::vec2& offset)
{
    m_Offset = offset;
    updateFilter();
}

glm::vec2 ShadowFXNode::getOffset() const
{
    return m_Offset;
}

void ShadowFXNode::setRadius(float radius)
{
    m_StdDev = radius;
    updateFilter();
}

float ShadowFXNode::getRadius() const
{
    return m_StdDev;
}

void ShadowFXNode::setOpacity(float opacity)
{
    m_Opacity = opacity;
    updateFilter();
}

float ShadowFXNode::getOpacity() const
{
    return m_Opacity;
}

void ShadowFXNode::setColor(const std::string& sColor)
{
    m_sColorName = sColor;
    m_Color = colorStringToColor(sColor);
    updateFilter();
}

std::string ShadowFXNode::getColor() const
{
    return m_sColorName;
}

GPUFilterPtr ShadowFXNode::createFilter(const IntPoint& size)
{
    m_pFilter = GPUShadowFilterPtr(new GPUShadowFilter(size, m_Offset, m_StdDev, 
            m_Opacity, m_Color));
    setDirty();
    return m_pFilter;
}

void ShadowFXNode::updateFilter()
{
    if (m_pFilter) {
        m_pFilter->setParams(m_Offset, m_StdDev, m_Opacity, m_Color);
        setDirty();
    }
}

}

