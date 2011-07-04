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

#ifndef _HslColorFXNode_H_
#define _HslColorFXNode_H_

#include <api.h>

#include "FXNode.h"
#include <graphics/GPUHslColorFilter.h>

#include <boost/shared_ptr.hpp>
#include <boost/python.hpp>
#include <string>

using namespace std;

namespace avg {
    class SDLDisplayEngine;

    class AVG_API HslColorFXNode : public FXNode {
    public:
            HslColorFXNode(float hue=0.0f, float saturation=1.0f, float lightness=0.0f,
                bool tint=false );
            virtual ~HslColorFXNode();
            virtual void disconnect();

            void setHSL(float hue, float saturation, float brightnessOffset);
            void setHue(float hue);
            void setSaturation(float saturation);
            void setBrightnessOffset(float brightnessOffset);
            void setColorizing(bool colorize);
            float getHue();
            float getSaturation();
            float getBrightnessOffset();
            bool isColorizing();

            std::string toString();

        private:
            virtual GPUFilterPtr createFilter(const IntPoint& size);
            void setFilterParams();
            GPUHslColorFilterPtr filterPtr;

            float m_fHue, m_fBrightnessOffset, m_fSaturation;
            bool m_bColorize;
    };

    typedef boost::shared_ptr<HslColorFXNode> HslColorFXNodePtr;
}
#endif
