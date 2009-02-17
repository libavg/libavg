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

OGLProgramPtr GPUBandpassFilter::s_pShader;

GPUBandpassFilter::GPUBandpassFilter(const IntPoint& size, PixelFormat pfSrc, 
        double min, double max, double postScale, bool bInvert)
    : GPUFilter(size, pfSrc, B8G8R8A8, true),
      m_PostScale(postScale),
      m_bInvert(bInvert),
      m_pMinPBO(new PBOImage(size, R32G32B32A32F, R32G32B32A32F, false, false)),
      m_pMaxPBO(new PBOImage(size, R32G32B32A32F, R32G32B32A32F, false, false)),
      m_MinFilter(getSrcPBO(), m_pMinPBO, min),
      m_MaxFilter(getSrcPBO(), m_pMaxPBO, max)
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

void GPUBandpassFilter::applyOnGPU()
{
    m_MinFilter.apply();
    m_MaxFilter.apply();

    getFBO()->activate();
    s_pShader->activate();
    s_pShader->setUniformIntParam("minTex",0);
    s_pShader->setUniformIntParam("maxTex",1);
    s_pShader->setUniformFloatParam("postScale", float(m_PostScale));
    s_pShader->setUniformIntParam("bInvert",m_bInvert);
    m_pMaxPBO->activateTex(GL_TEXTURE1);
    m_pMinPBO->draw();

    glproc::UseProgramObject(0);
}

void GPUBandpassFilter::initShader()
{
    string sProgram =
        "#extension GL_ARB_texture_rectangle : enable\n" 
       
        "uniform sampler2DRect minTex;\n"
        "uniform sampler2DRect maxTex;\n"
        "uniform float postScale;\n"
        "uniform bool bInvert;\n"

        "void main(void)\n"
        "{\n"
        "  vec4 min =texture2DRect(minTex, gl_TexCoord[0].st);\n" 
        "  vec4 max =texture2DRect(maxTex, gl_TexCoord[0].st);\n"
        "  gl_FragColor = vec4(0.502, 0.502, 0.502, 0)+(max-min)*postScale;\n"
        "  if (bInvert) {\n"
        "    gl_FragColor = vec4(1.004,1.004,1.004,1)-gl_FragColor;\n"
        "  }\n"
        "  gl_FragColor.a = 1.0;\n"
        "}\n"
        ;

    s_pShader = OGLProgram::buildProgram(OGLShaderPtr(new OGLShader(sProgram)));
}

} // namespace
