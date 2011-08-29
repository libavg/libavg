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

#ifndef _GPUShadowFilter_H_
#define _GPUShadowFilter_H_

#include "../api.h"
#include "GPUFilter.h"
#include "GLTexture.h"

namespace avg {

class AVG_API GPUShadowFilter: public GPUFilter
{
public:
    GPUShadowFilter(const IntPoint& size, const DPoint& offset, double stdDev,
            double opacity, const Pixel32& color);
    virtual ~GPUShadowFilter();
    
    void setParams(const DPoint& offset, double stdDev, double opacity, 
            const Pixel32& color);
    virtual void applyOnGPU(GLTexturePtr pSrcTex);

private:
    void initShaders();
    void dumpKernel();
    void calcKernel();
    void setDimensions(IntPoint size, double stdDev, const DPoint& offset);

    DPoint m_Offset;
    double m_StdDev;
    double m_Opacity;
    Pixel32 m_Color;

    GLTexturePtr m_pGaussCurveTex;
    ImagingProjectionPtr m_pProjection2;
};

typedef boost::shared_ptr<GPUShadowFilter> GPUShadowFilterPtr;

} // namespace
#endif

