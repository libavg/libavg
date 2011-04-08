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

#include "GPUBrightnessFilter.h"
#include "Bitmap.h"
#include "ShaderRegistry.h"

#include "../base/ObjectCounter.h"
#include "../base/Exception.h"

#include <iostream>

#define SHADERID "BRIGHTNESS"

using namespace std;

namespace avg {

GPUBrightnessFilter::GPUBrightnessFilter(const IntPoint& size, PixelFormat pf, 
        double alpha, bool bStandalone)
    : GPUFilter(pf, B8G8R8A8, bStandalone),
      m_Alpha(alpha)
{
    ObjectCounter::get()->incRef(&typeid(*this));
    setDimensions(size);
    initShader();
}

GPUBrightnessFilter::~GPUBrightnessFilter()
{
    ObjectCounter::get()->decRef(&typeid(*this));
}

void GPUBrightnessFilter::applyOnGPU(GLTexturePtr pSrcTex)
{
    OGLShaderPtr pShader = getShader(SHADERID);
    pShader->activate();
    pShader->setUniformIntParam("Texture", 0);
    pShader->setUniformFloatParam("alpha", GLfloat(m_Alpha));
    draw(pSrcTex);

    glproc::UseProgramObject(0);
}

void GPUBrightnessFilter::initShader()
{
    string sProgram =
        "uniform float alpha;\n"
        "uniform sampler2D Texture;\n"

        "void main(void)\n"
        "{\n"
        "    vec4 tex =texture2D(Texture, gl_TexCoord[0].st);\n" 
        "    gl_FragColor.rgb = tex.rgb*alpha;\n"
        "    gl_FragColor.a = tex.a;\n"
        "}\n"
        ;

    getOrCreateShader(SHADERID, sProgram);
}

} // namespace
