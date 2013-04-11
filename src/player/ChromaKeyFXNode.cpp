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

#include "ChromaKeyFXNode.h"

#include "../base/ObjectCounter.h"

#include <string>

using namespace std;

namespace avg {

ChromaKeyFXNode::ChromaKeyFXNode() 
    : FXNode(false),
      m_sColorName("00FF00"),
      m_Color(0, 255, 0),
      m_HTolerance(0.0),
      m_STolerance(0.0),
      m_LTolerance(0.0),
      m_Softness(0.0),
      m_Erosion(0),
      m_SpillThreshold(0.0)
{
    ObjectCounter::get()->incRef(&typeid(*this));
}

ChromaKeyFXNode::~ChromaKeyFXNode()
{
    ObjectCounter::get()->decRef(&typeid(*this));
}

void ChromaKeyFXNode::disconnect()
{
    m_pFilter = GPUChromaKeyFilterPtr();
    FXNode::disconnect();
}
    

void ChromaKeyFXNode::setColor(const std::string& sColorName)
{
    m_sColorName = sColorName;
    m_Color = colorStringToColor(m_sColorName);
    updateFilter();
}

const std::string& ChromaKeyFXNode::getColor() const
{
    return m_sColorName;
}

void ChromaKeyFXNode::setHTolerance(float tolerance)
{
    m_HTolerance = tolerance;
    updateFilter();
}

float ChromaKeyFXNode::getHTolerance() const
{
    return m_HTolerance;
}

void ChromaKeyFXNode::setSTolerance(float tolerance)
{
    m_STolerance = tolerance;
    updateFilter();
}

float ChromaKeyFXNode::getSTolerance() const
{
    return m_STolerance;
}

void ChromaKeyFXNode::setLTolerance(float tolerance)
{
    m_LTolerance = tolerance;
    updateFilter();
}

float ChromaKeyFXNode::getLTolerance() const
{
    return m_LTolerance;
}

void ChromaKeyFXNode::setSoftness(float softness)
{
    m_Softness = softness;
    updateFilter();
}

float ChromaKeyFXNode::getSoftness() const
{
    return m_Softness;
}

void ChromaKeyFXNode::setErosion(int erosion)
{
    m_Erosion = erosion;
    updateFilter();
}

int ChromaKeyFXNode::getErosion() const
{
    return m_Erosion;
}

void ChromaKeyFXNode::setSpillThreshold(float spillThreshold)
{
    m_SpillThreshold = spillThreshold;
    updateFilter();
}

float ChromaKeyFXNode::getSpillThreshold() const
{
    return m_SpillThreshold;
}

GPUFilterPtr ChromaKeyFXNode::createFilter(const IntPoint& size)
{
    m_pFilter = GPUChromaKeyFilterPtr(new GPUChromaKeyFilter(size, false));
    m_pFilter->setParams(m_Color, m_HTolerance, m_STolerance, m_LTolerance, m_Softness,
            m_Erosion, m_SpillThreshold);
    setDirty();
    return m_pFilter;
}

void ChromaKeyFXNode::updateFilter()
{
    if (m_pFilter) {
        m_pFilter->setParams(m_Color, m_HTolerance, m_STolerance, m_LTolerance, 
                m_Softness, m_Erosion, m_SpillThreshold);
        setDirty();
    }
}

}
