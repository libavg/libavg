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

#include "ChromaKeyFXNode.h"
#include "SDLDisplayEngine.h"

#include "../base/ObjectCounter.h"
#include "../graphics/ShaderRegistry.h"

#include <string>

using namespace std;

namespace avg {

ChromaKeyFXNode::ChromaKeyFXNode() 
    : FXNode(),
      m_sColorName("00FF00"),
      m_Color(0, 255, 0),
      m_HTolerance(0.0),
      m_STolerance(0.0),
      m_LTolerance(0.0),
      m_Softness(0.0)
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
    setFilterParams();
}

const std::string& ChromaKeyFXNode::getColor() const
{
    return m_sColorName;
}

void ChromaKeyFXNode::setHTolerance(double tolerance)
{
    m_HTolerance = tolerance;
    setFilterParams();
}

double ChromaKeyFXNode::getHTolerance() const
{
    return m_HTolerance;
}

void ChromaKeyFXNode::setSTolerance(double tolerance)
{
    m_STolerance = tolerance;
    setFilterParams();
}

double ChromaKeyFXNode::getSTolerance() const
{
    return m_STolerance;
}

void ChromaKeyFXNode::setLTolerance(double tolerance)
{
    m_LTolerance = tolerance;
    setFilterParams();
}

double ChromaKeyFXNode::getLTolerance() const
{
    return m_LTolerance;
}

void ChromaKeyFXNode::setSoftness(double softness)
{
    m_Softness = softness;
    setFilterParams();
}

double ChromaKeyFXNode::getSoftness() const
{
    return m_Softness;
}

GPUFilterPtr ChromaKeyFXNode::createFilter(const IntPoint& size)
{
    m_pFilter = GPUChromaKeyFilterPtr(new GPUChromaKeyFilter(size, B8G8R8A8, false));
    m_pFilter->setParams(m_Color, m_HTolerance, m_STolerance, m_LTolerance, m_Softness);
    return m_pFilter;
}

void ChromaKeyFXNode::setFilterParams()
{
    if (m_pFilter) {
        m_pFilter->setParams(m_Color, m_HTolerance, m_STolerance, m_LTolerance, 
                m_Softness);
    }
    
}

}
