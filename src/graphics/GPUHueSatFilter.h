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

#ifndef _GPUHueSatFilter_H_
#define _GPUHueSatFilter_H_

#include "../api.h"

#include "GPUFilter.h"

namespace avg {

class AVG_API GPUHueSatFilter : public GPUFilter
{
public:
    GPUHueSatFilter(const IntPoint& size, PixelFormat pf,
            bool bStandalone=true);
    virtual ~GPUHueSatFilter();

    virtual void applyOnGPU(GLTexturePtr pSrcTex);
    void initShader();
    void setParams(int hue, int saturation=1, int lightness_offset=0,
            bool colorize=false);

private:
    float m_fLightnessOffset;
    float m_fHue;
    float m_fSaturation;
    bool m_bColorize;
};

typedef boost::shared_ptr<GPUHueSatFilter> GPUHueSatFilterPtr;

} //end namespace avg
#endif
