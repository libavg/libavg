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

#ifndef _GPUChromaKeyFilter_H_
#define _GPUChromaKeyFilter_H_

#include "../api.h"
#include "GPUFilter.h"
#include "Bitmap.h"

namespace avg {

class AVG_API GPUChromaKeyFilter: public GPUFilter
{
public:
    GPUChromaKeyFilter(const IntPoint& size, PixelFormat pf, bool bStandalone=true);
    virtual ~GPUChromaKeyFilter();

    void setParams(const Pixel32& color, double hTolerance, double sTolerance, 
            double lTolerance, double softness, int erosion, double spillThreshold);

    virtual void applyOnGPU(GLTexturePtr pSrcTex);

private:
    static void initShader();

    Pixel32 m_Color;
    double m_HTolerance;
    double m_STolerance;
    double m_LTolerance;
    double m_Softness;
    int m_Erosion;
    double m_SpillThreshold;
};

typedef boost::shared_ptr<GPUChromaKeyFilter> GPUChromaKeyFilterPtr;

}
#endif

