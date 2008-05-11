//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2006 Ulrich von Zadow
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

GPUBandpassFilter::GPUBandpassFilter(const IntPoint& size, PixelFormat pf, 
        double min, double max)
    : GPUFilter(size, pf),
      m_pMinFBO(new FBOImage(size, B8G8R8A8)),
      m_pMaxFBO(new FBOImage(size, B8G8R8A8)),
      m_MinFilter(getSrcPBO(), m_pMinFBO, min),
      m_MaxFilter(getSrcPBO(), m_pMaxFBO, max)
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
    m_MinFilter.applyOnGPU();
    m_MaxFilter.applyOnGPU();

    getDestFBO()->activate();
    GLhandleARB hProgram = s_pShader->getProgram();
    glproc::UseProgramObject(hProgram);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            "GPUBandpassFilter::apply: glUseProgramObject()");
    glproc::Uniform1i(glproc::GetUniformLocation(hProgram, "minTex"), 0);
    glproc::Uniform1i(glproc::GetUniformLocation(hProgram, "maxTex"), 1);
    m_pMaxFBO->activateTex(GL_TEXTURE1);
    m_pMinFBO->draw();

    glproc::UseProgramObject(0);
    getDestFBO()->deactivate();
}

void GPUBandpassFilter::initShader()
{
    string sProgram =
        "#extension GL_ARB_texture_rectangle : enable\n" 
        
        "uniform sampler2DRect minTex;\n"
        "uniform sampler2DRect maxTex;\n"

        "void main(void)\n"
        "{\n"
        "  vec4 min =texture2DRect(minTex, gl_TexCoord[0].st);\n" 
        "  vec4 max =texture2DRect(maxTex, gl_TexCoord[0].st);\n" 
        "  gl_FragColor = vec4(0.504, 0.504, 0.504, 0)+max-min;\n"
        "  gl_FragColor.a = 1.0;\n"
        "}\n"
        ;

    s_pShader = OGLShaderPtr(new OGLShader(sProgram));
}

} // namespace
