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

#include "HslColorFXNode.h"
#include "SDLDisplayEngine.h"

#include <base/ObjectCounter.h>
#include <base/Logger.h>
#include <graphics/ShaderRegistry.h>

#include<sstream>

using namespace std;

namespace avg {

    HslColorFXNode::HslColorFXNode(float hue,float saturation, float lightness,
        bool tint):
        FXNode(),
        m_fHue(hue),
        m_fBrightnessOffset(lightness),
        m_fSaturation(saturation),
        m_bColorize(tint)
    {
        ObjectCounter::get()->incRef(&typeid(*this));
    }

    HslColorFXNode::~HslColorFXNode() {
        ObjectCounter::get()->decRef(&typeid(*this));
    }

    void HslColorFXNode::disconnect() {
        filterPtr = GPUHslColorFilterPtr();
        FXNode::disconnect();
    }

    void HslColorFXNode::setHSL(float hue, float saturation, float brightnessOffset) {
        setHue(hue);
        setSaturation(saturation);
        setBrightnessOffset(brightnessOffset);
    }

    float HslColorFXNode::getHue() {
        return m_fHue;
    }

    float HslColorFXNode::getSaturation() {
        return m_fSaturation;
    }

    float HslColorFXNode::getBrightnessOffset() {
        return m_fBrightnessOffset;
    }

    bool HslColorFXNode::isColorizing(){
        return m_bColorize;
    }

    void HslColorFXNode::setHue(float hue) {
        m_fHue = hue;
        setFilterParams();
    }

    void HslColorFXNode::setSaturation(float saturation) {
        m_fSaturation = saturation;
        setFilterParams();
    }

    void HslColorFXNode::setBrightnessOffset(float brightnessOffset) {
        m_fBrightnessOffset = brightnessOffset;
        setFilterParams();
    }

    void HslColorFXNode::setColorizing(bool colorize){
        m_bColorize = colorize;
        setFilterParams();
    }

    GPUFilterPtr HslColorFXNode::createFilter(const IntPoint& size) {
        filterPtr = GPUHslColorFilterPtr(new GPUHslColorFilter(size, B8G8R8A8,
                false));
        setFilterParams();
        return filterPtr;
    }

    void HslColorFXNode::setFilterParams() {
        if (filterPtr) {
            filterPtr->setParams(m_fHue, m_fSaturation, m_fBrightnessOffset, m_bColorize);
        }
    }

    std::string HslColorFXNode::toString() {
        stringstream s;
        s << "HslColorFXNode( " << m_fHue << ", " << m_fSaturation << ", "
            << m_fBrightnessOffset << " )";
        return s.str();
    }
} //End namespace avg

