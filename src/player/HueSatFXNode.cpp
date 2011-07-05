//
////  libavg - Media Playback Engine. 
////  Copyright (C) 2003-2011 Ulrich von Zadow
////
////  This library is free software; you can redistribute it and/or
////  modify it under the terms of the GNU Lesser General Public
////  License as published by the Free Software Foundation; either
////  version 2 of the License, or (at your option) any later version.
////
////  This library is distributed in the hope that it will be useful,
////  but WITHOUT ANY WARRANTY; without even the implied warranty of
////  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
////  Lesser General Public License for more details.
////
////  You should have received a copy of the GNU Lesser General Public
////  License along with this library; if not, write to the Free Software
////  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
////
////  Current versions can be found at www.libavg.de
////
//

#include "HueSatFXNode.h"
#include "SDLDisplayEngine.h"

#include <base/ObjectCounter.h>
#include <base/Logger.h>
#include <graphics/ShaderRegistry.h>

#include<sstream>

using namespace std;

namespace avg {

HueSatFXNode::HueSatFXNode(float hue,float saturation, float lightness,
    bool tint):
    FXNode(),
    m_fHue(hue),
    m_fLightnessOffset(lightness),
    m_fSaturation(saturation),
    m_bColorize(tint)
{
    ObjectCounter::get()->incRef(&typeid(*this));
}

HueSatFXNode::~HueSatFXNode() {
    ObjectCounter::get()->decRef(&typeid(*this));
}

void HueSatFXNode::disconnect()
{
    filterPtr = GPUHueSatFilterPtr();
    FXNode::disconnect();
}

void HueSatFXNode::setHSL(float hue, float saturation, float lightnessOffset)
{
    setHue(hue);
    setSaturation(saturation);
    setLightnessOffset(lightnessOffset);
}

float HueSatFXNode::getHue()
{
    return m_fHue;
}

float HueSatFXNode::getSaturation()
{
    return m_fSaturation;
}

float HueSatFXNode::getLightnessOffset()
{
    return m_fLightnessOffset;
}

bool HueSatFXNode::isColorizing()
{
    return m_bColorize;
}

void HueSatFXNode::setHue(float hue)
{
    m_fHue = hue;
    setFilterParams();
}

void HueSatFXNode::setSaturation(float saturation)
{
    m_fSaturation = saturation;
    setFilterParams();
}

void HueSatFXNode::setLightnessOffset(float lightnessOffset)
{
    m_fLightnessOffset = lightnessOffset;
    setFilterParams();
}

void HueSatFXNode::setColorizing(bool colorize)
{
    m_bColorize = colorize;
    setFilterParams();
}

GPUFilterPtr HueSatFXNode::createFilter(const IntPoint& size)
{
    filterPtr = GPUHueSatFilterPtr(new GPUHueSatFilter(size, B8G8R8A8,
            false));
    setFilterParams();
    return filterPtr;
}

void HueSatFXNode::setFilterParams()
{
    if (filterPtr) {
        filterPtr->setParams(m_fHue, m_fSaturation, m_fLightnessOffset, m_bColorize);
    }
}

std::string HueSatFXNode::toString()
{
    stringstream s;
    s << "HueSatFXNode( " << m_fHue << ", " << m_fSaturation << ", "
        << m_fLightnessOffset << " )";
    return s.str();
}
} //End namespace avg

