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

#include "../base/ObjectCounter.h"
#include "../base/Exception.h"

#include <iostream>

using namespace std;

namespace avg {

OGLShaderPtr GPUBrightnessFilter::s_pShader;

GPUBrightnessFilter::GPUBrightnessFilter(const IntPoint& size, PixelFormat pf, 
        double alpha, bool bStandalone)
    : GPUFilter(size, pf, B8G8R8A8, bStandalone),
      m_Alpha(alpha)
{
    ObjectCounter::get()->incRef(&typeid(*this));

    if (!s_pShader) {
        initShader();
    }
}

GPUBrightnessFilter::~GPUBrightnessFilter()
{
    ObjectCounter::get()->decRef(&typeid(*this));
}

void GPUBrightnessFilter::applyOnGPU(GLTexturePtr pSrcTex)
{
    GLhandleARB hProgram = s_pShader->getProgram();
    glproc::UseProgramObject(hProgram);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            "GPUBrightnessFilter::apply: glUseProgramObject()");
    glproc::Uniform1f(glproc::GetUniformLocation(hProgram, "alpha"),
            GLfloat(m_Alpha));
    glproc::Uniform1i(glproc::GetUniformLocation(hProgram, "Texture"), 0);
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
        "  vec4 tex =texture2D(Texture, gl_TexCoord[0].st);\n" 
        "  gl_FragColor.rgb = tex.rgb*alpha;\n"
        "  gl_FragColor.a = tex.a;\n"
        "}\n"
        ;

    s_pShader = OGLShaderPtr(new OGLShader(sProgram));
}

} // namespace
