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

#include "GPUBandpassFilter.h"
#include "Bitmap.h"

#include "../base/ObjectCounter.h"
#include "../base/Exception.h"

#include <iostream>

using namespace std;

namespace avg {

OGLShaderPtr GPUBandpassFilter::s_pShader;

GPUBandpassFilter::GPUBandpassFilter(const IntPoint& size, PixelFormat pfSrc, 
        double min, double max, double postScale, bool bInvert, bool bStandalone)
    : GPUFilter(size, pfSrc, B8G8R8A8, bStandalone),
      m_PostScale(postScale),
      m_bInvert(bInvert),
      m_MinFilter(size, pfSrc, min, false),
      m_MaxFilter(size, pfSrc, max, false)
{
    ObjectCounter::get()->incRef(&typeid(*this));

    if (!s_pShader) {
        initShader();
    }
}

GPUBandpassFilter::~GPUBandpassFilter()
{
    ObjectCounter::get()->decRef(&typeid(*this));
}

void GPUBandpassFilter::applyOnGPU(GLTexturePtr pSrcTex)
{
    m_MinFilter.apply(pSrcTex);
    m_MaxFilter.apply(pSrcTex);

    getFBO()->activate();
    GLhandleARB hProgram = s_pShader->getProgram();
    glproc::UseProgramObject(hProgram);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            "GPUBandpassFilter::apply: glUseProgramObject()");
    glproc::Uniform1i(glproc::GetUniformLocation(hProgram, "minTex"), 0);
    glproc::Uniform1i(glproc::GetUniformLocation(hProgram, "maxTex"), 1);
    glproc::Uniform1f(glproc::GetUniformLocation(hProgram, "postScale"), 
            float(m_PostScale));
    glproc::Uniform1i(glproc::GetUniformLocation(hProgram, "bInvert"), m_bInvert);
    m_MaxFilter.getDestTex()->activate(GL_TEXTURE1);
    draw(m_MinFilter.getDestTex());

    glproc::UseProgramObject(0);
}

void GPUBandpassFilter::initShader()
{
    string sProgram =
        "uniform sampler2D minTex;\n"
        "uniform sampler2D maxTex;\n"
        "uniform float postScale;\n"
        "uniform bool bInvert;\n"

        "void main(void)\n"
        "{\n"
        "  vec4 min = texture2D(minTex, gl_TexCoord[0].st);\n" 
        "  vec4 max = texture2D(maxTex, gl_TexCoord[0].st);\n"
        "  gl_FragColor = vec4(0.502, 0.502, 0.502, 0)+(max-min)*postScale;\n"
        "  if (bInvert) {\n"
        "    gl_FragColor = vec4(1.004,1.004,1.004,1)-gl_FragColor;\n"
        "  }\n"
        "  gl_FragColor.a = 1.0;\n"
        "}\n"
        ;

    s_pShader = OGLShaderPtr(new OGLShader(sProgram));
}

} // namespace
