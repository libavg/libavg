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

#include "GPUHueSatFilter.h"
#include "ShaderRegistry.h"

#include "../base/ObjectCounter.h"
#include "../base/Logger.h"

#define SHADERID_HSL_COLOR "HSL_COLOR"

using namespace std;

namespace avg {

GPUHueSatFilter::GPUHueSatFilter(const IntPoint& size, PixelFormat pf,
        bool bStandalone) :
    GPUFilter(pf, B8G8R8A8, bStandalone, 2),
    m_LightnessOffset(0.0),
    m_Hue(0.0),
    m_Saturation(0.0)
{
    ObjectCounter::get()->incRef(&typeid(*this));
    setDimensions(size);
    initShader();
}

GPUHueSatFilter::~GPUHueSatFilter()
{
    ObjectCounter::get()->decRef(&typeid(*this));
}

void GPUHueSatFilter::setParams(int hue, int saturation,
        int light_add, bool colorize)
{
    m_Hue = float(hue);
    m_Saturation = saturation / 100.0f;
    m_LightnessOffset = light_add / 100.0f;
    m_bColorize = colorize;
}

void GPUHueSatFilter::applyOnGPU(GLTexturePtr pSrcTex)
{
    OGLShaderPtr pShader = getShader(SHADERID_HSL_COLOR);
    glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);
    pShader->activate();
    pShader->setUniformFloatParam("hue", m_Hue);
    pShader->setUniformFloatParam("sat", m_Saturation);
    pShader->setUniformFloatParam("l_offset", m_LightnessOffset);
    pShader->setUniformIntParam("b_colorize", (int)m_bColorize);
    pShader->setUniformIntParam("texture", 0);
    draw(pSrcTex);
    glproc::UseProgramObject(0);
}

void GPUHueSatFilter::initShader()
{
    string sProgramHead =
            "const vec3 lumCoeff = vec3(0.2125, 0.7154, 0.0721);\n"
            "const vec3 white = vec3(1.0, 1.0, 1.0);\n"
            "const vec3 black = vec3(0.0, 0.0, 0.0);\n"
            "uniform sampler2D texture;\n"
            "uniform float hue;\n"
            "uniform float sat;\n"
            "uniform float l_offset;\n"
            "uniform bool b_colorize;\n"
            + getStdShaderCode()
            ;
    string sProgram = sProgramHead +
            "void main(void)\n"
            "{\n"
            "    float tmp;\n"
            "    float s;\n"
            "    float l;\n"
            "    float h;\n"
            "    vec4 tex = texture2D(texture, gl_TexCoord[0].st);\n"
            "    unPreMultiplyAlpha(tex);\n"
            "    rgb2hsl(tex, tmp, s, l);\n"
            "    if(b_colorize){\n"
            "       h = hue;\n"
            "       s = sat;\n"
            "    }\n"
            "    else{\n"
            "       h = hue+tmp;\n"
            "    }\n"
            "    vec4 rgbTex = vec4(hsl2rgb(mod(h, 360.0), s, l), tex.a);\n"
            //Saturate in rgb - space to imitate photoshop filter
            "    if(!b_colorize){ \n"
            "      s = clamp(sat+s, 0.0, 2.0);\n"
            "      vec3 intensity = vec3(dot(rgbTex.rgb, lumCoeff));\n"
            "      rgbTex.rgb = mix(intensity, rgbTex.rgb, s);\n"
            "    }; \n"
            //Brightness with black/white pixels to imitate photoshop lightness-offset
            "    if(l_offset >= 0.0){ \n"
            "       rgbTex = vec4(mix(rgbTex.rgb, white, l_offset), tex.a);\n"
            "    }\n"
            "    else if(l_offset < 0.0){ \n"
            "       rgbTex = vec4(mix(rgbTex.rgb, black, -l_offset), tex.a);\n"
            "    } \n"
            "    preMultiplyAlpha(rgbTex);\n"
            "    gl_FragColor = rgbTex;\n"
            "}\n";
    getOrCreateShader(SHADERID_HSL_COLOR, sProgram);
}
}//End namespace avg

