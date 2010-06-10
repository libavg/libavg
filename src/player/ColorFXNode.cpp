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

#include "ColorFXNode.h"
#include "SDLDisplayEngine.h"

#include "../base/ObjectCounter.h"

#include <string>

using namespace std;

namespace avg {

OGLShaderPtr ColorFXNode::s_pShader;

ColorFXNode::ColorFXNode() 
    : FXNode(),
      m_Brightness(1),
      m_Contrast(1),
      m_RGamma(1),
      m_GGamma(1),
      m_BGamma(1)
{
    ObjectCounter::get()->incRef(&typeid(*this));
}

ColorFXNode::~ColorFXNode()
{
    ObjectCounter::get()->decRef(&typeid(*this));
}

void ColorFXNode::connect(SDLDisplayEngine* pEngine)
{
    FXNode::connect(pEngine);
    initShader();
}

void ColorFXNode::disconnect()
{
    FXNode::disconnect();
}

void ColorFXNode::setSize(const IntPoint& newSize)
{
    FXNode::setSize(newSize);
}

void ColorFXNode::setParams(float brightness, float contrast, float rGamma, 
        float gGamma, float bGamma)
{
    m_Brightness = brightness;
    m_Contrast = contrast;
    m_RGamma = rGamma;
    m_GGamma = gGamma;
    m_BGamma = bGamma;
}

void ColorFXNode::apply(GLTexturePtr pSrcTex)
{
    s_pShader->activate();
    s_pShader->setUniformIntParam("texture", 0);
    s_pShader->setUniformFloatParam("brightness", m_Brightness);
    s_pShader->setUniformFloatParam("contrast", m_Contrast);
    s_pShader->setUniformFloatParam("rGamma", m_RGamma);
    s_pShader->setUniformFloatParam("gGamma", m_GGamma);
    s_pShader->setUniformFloatParam("bGamma", m_BGamma);

    // blt overwrites everything, so no glClear necessary before.
    getEngine()->setBlendMode(DisplayEngine::BLEND_COPY);
    FBOPtr pFBO = getFBO();
    pSrcTex->activate();
    pFBO->activate();
    pFBO->setupImagingProjection();
    pFBO->drawImagingVertexes();
    pFBO->deactivate();
    glproc::UseProgramObject(0);
    pFBO->copyToDestTexture();
}

void ColorFXNode::initShader()
{
//    if (!s_pShader) {
        string sProgram =
            "uniform sampler2D texture;\n"
            "uniform float brightness;\n"
            "uniform float contrast;\n"
            "uniform float rGamma;\n"
            "uniform float gGamma;\n"
            "uniform float bGamma;\n"
            "void main(void)\n"
            "{\n"
            "  vec4 tex = texture2D(texture, gl_TexCoord[0].st);\n"
            "  vec3 avg = vec3(0.5, 0.5, 0.5);\n"
            "  tex.rgb = mix(avg, tex.rgb, contrast);\n"
            "  tex.rgb = tex.rgb*brightness;\n"
            "  gl_FragColor = tex;\n"
            "}\n"
            ;

        s_pShader = OGLShaderPtr(new OGLShader(sProgram));
//    }
}

}
