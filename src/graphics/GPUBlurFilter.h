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

#ifndef _GPUBlurFilter_H_
#define _GPUBlurFilter_H_

#include "GPUFilter.h"
#include "PBOImage.h"
#include "FBOImage.h"
#include "OGLShader.h"

namespace avg {

class GPUBlurFilter: public GPUFilter
{
public:
    GPUBlurFilter(const IntPoint& size, PixelFormat pfSrc, double stdDev);
    GPUBlurFilter(PBOImagePtr pSrcPBO, FBOImagePtr pDestFBO, double stdDev);
    void init();
    virtual ~GPUBlurFilter();

    virtual void applyOnGPU();

private:
    static void initShaders();
    void dumpKernel();
    void calcKernel();

    double m_StdDev;
    int m_KernelWidth;
    float m_Kernel[255];

    PBOImagePtr m_pGaussCurvePBO;
    FBOImagePtr m_pInterFBO;

    static OGLShaderPtr s_pHorizShader;
    static OGLShaderPtr s_pVertShader;
};

} // namespace
#endif

