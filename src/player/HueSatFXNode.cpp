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


#include "HueSatFXNode.h"

#include "../base/ObjectCounter.h"
#include "../base/Logger.h"
#include "../graphics/ShaderRegistry.h"

#include<sstream>

using namespace std;

namespace avg {

HueSatFXNode::HueSatFXNode(int hue, int saturation, int lightness,
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

int HueSatFXNode::getHue()
{
    if(m_bColorize){
        if( m_fHue < 0){
            return 360 + m_fHue;
        }
        return m_fHue;
    }

    if((m_fHue / 180.0) > 1.0)
    {
        return -360+m_fHue;
    }else if((m_fHue / 180.0) < -1.0)
    {
        return 360+m_fHue;
    }
    return m_fHue;
}

int HueSatFXNode::getSaturation()
{
    return m_fSaturation;
}

int HueSatFXNode::getLightnessOffset()
{
    return m_fLightnessOffset;
}

bool HueSatFXNode::isColorizing()
{
    return m_bColorize;
}

void HueSatFXNode::setHue(int hue)
{
    m_fHue = hue % 360;
    setFilterParams();
}

void HueSatFXNode::setSaturation(int saturation)
{
    if(m_bColorize){
        m_fSaturation = clamp(saturation, 0, 100);
    }else{
        m_fSaturation = clamp(saturation, -100, 100); 
    }
    setFilterParams();
}

void HueSatFXNode::setLightnessOffset(int lightnessOffset)
{
    m_fLightnessOffset= clamp(lightnessOffset, -100, 100);
    setFilterParams();
}

void HueSatFXNode::setColorizing(bool colorize)
{
    m_bColorize = colorize;
    m_fHue = 0;
    m_fLightnessOffset = 0;
    m_fSaturation = m_bColorize ? 50 : 0;
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
    s << "HueSatFXNode( Hue: " << m_fHue << ", Saturation: " << m_fSaturation << ", Lightness: "
        << m_fLightnessOffset << ", Colorize: " << m_bColorize << " )";
    return s.str();
}

int HueSatFXNode::clamp(int val, int min, int max)
{
    int result = val;
    if(val < min){
        result = min;
    }else if(val > max){
        result = max;
    }
    return result;
}
} //End namespace avg

