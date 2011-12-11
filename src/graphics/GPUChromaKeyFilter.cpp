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

#include "GPUChromaKeyFilter.h"
#include "Bitmap.h"
#include "ShaderRegistry.h"

#include "../base/ObjectCounter.h"
#include "../base/Exception.h"

#include <iostream>

#define SHADERID_CHROMAKEY "chromakey"
#define SHADERID_EROSION "chromakey_erosion"

using namespace std;

namespace avg {

GPUChromaKeyFilter::GPUChromaKeyFilter(const IntPoint& size, PixelFormat pf, 
        bool bStandalone)
    : GPUFilter(pf, B8G8R8A8, bStandalone, 2),
      m_Color(0, 255, 0),
      m_HTolerance(0.0),
      m_STolerance(0.0),
      m_LTolerance(0.0),
      m_Softness(0.0),
      m_Erosion(0),
      m_SpillThreshold(0.0)
{
    ObjectCounter::get()->incRef(&typeid(*this));

    setDimensions(size);
    createShader(SHADERID_CHROMAKEY);
    createShader(SHADERID_EROSION);
}

GPUChromaKeyFilter::~GPUChromaKeyFilter()
{
    ObjectCounter::get()->decRef(&typeid(*this));
}

void GPUChromaKeyFilter::setParams(const Pixel32& color, float hTolerance, 
        float sTolerance, float lTolerance, float softness, int erosion,
        float spillThreshold)
{
    m_Color = color;
    m_HTolerance = hTolerance;
    m_STolerance = sTolerance;
    m_LTolerance = lTolerance;
    m_Softness = softness;
    m_Erosion = erosion;
    m_SpillThreshold = spillThreshold;
    if (m_SpillThreshold <= m_HTolerance) {
        m_SpillThreshold = m_HTolerance;
    }
}

void GPUChromaKeyFilter::applyOnGPU(GLTexturePtr pSrcTex)
{
    // Set up double-buffering
    int curBufferIndex = m_Erosion%2;
    OGLShaderPtr pShader = getShader(SHADERID_CHROMAKEY);
    glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT+curBufferIndex);
    pShader->activate();
    pShader->setUniformIntParam("texture", 0);

    float h, s, l;
    m_Color.toHSL(h, s, l);
    pShader->setUniformFloatParam("hKey", h);
    pShader->setUniformFloatParam("hTolerance", m_HTolerance*360);
    pShader->setUniformFloatParam("hSoftTolerance", (m_HTolerance+m_Softness)*360.0f);
    pShader->setUniformFloatParam("sKey", s);
    pShader->setUniformFloatParam("sTolerance", m_STolerance);
    pShader->setUniformFloatParam("sSoftTolerance", m_STolerance+m_Softness);
    pShader->setUniformFloatParam("lKey", l);
    pShader->setUniformFloatParam("lTolerance", m_LTolerance);
    pShader->setUniformFloatParam("lSoftTolerance", m_LTolerance+m_Softness);
    pShader->setUniformFloatParam("spillThreshold", m_SpillThreshold*360);
    pShader->setUniformIntParam("bIsLast", int(m_Erosion==0));
    draw(pSrcTex);

    for (int i = 0; i < m_Erosion; ++i) {
        curBufferIndex = (curBufferIndex+1)%2;
        glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT+curBufferIndex);
        pShader = getShader(SHADERID_EROSION);
        pShader->activate();
        pShader->setUniformIntParam("texture", 0);
        pShader->setUniformIntParam("bIsLast", int(i==m_Erosion-1));
        draw(getDestTex((curBufferIndex+1)%2));
    }
    glproc::UseProgramObject(0);
}

}
