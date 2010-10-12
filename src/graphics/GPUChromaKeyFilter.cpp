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

#include "GPUChromaKeyFilter.h"
#include "Bitmap.h"
#include "ShaderRegistry.h"

#include "../base/ObjectCounter.h"
#include "../base/Exception.h"

#include <iostream>

#define SHADERID_CHROMAKEY "CHROMAKEY"
#define SHADERID_EROSION "CHROMAKEY_EROSION"

using namespace std;

namespace avg {

GPUChromaKeyFilter::GPUChromaKeyFilter(const IntPoint& size, PixelFormat pf, 
        bool bStandalone)
    : GPUFilter(size, pf, B8G8R8A8, bStandalone, 2),
      m_Color(0, 255, 0),
      m_HTolerance(0.0),
      m_STolerance(0.0),
      m_LTolerance(0.0),
      m_Softness(0.0),
      m_Erosion(0),
      m_SpillThreshold(0.0)
{
    ObjectCounter::get()->incRef(&typeid(*this));

    initShader();
}

GPUChromaKeyFilter::~GPUChromaKeyFilter()
{
    ObjectCounter::get()->decRef(&typeid(*this));
}

void GPUChromaKeyFilter::setParams(const Pixel32& color, double hTolerance, 
        double sTolerance, double lTolerance, double softness, int erosion,
        double spillThreshold)
{
    m_Color = color;
    m_HTolerance = hTolerance;
    m_STolerance = sTolerance;
    m_LTolerance = lTolerance;
    m_Softness = softness;
    m_Erosion = erosion;
    m_SpillThreshold = spillThreshold;
}

void GPUChromaKeyFilter::applyOnGPU(GLTexturePtr pSrcTex)
{
    // Set up double-buffering
    int curBufferIndex = m_Erosion%2;
    OGLShaderPtr pShader = getShader(SHADERID_CHROMAKEY);
    glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT+curBufferIndex);
    pShader->activate();
    pShader->setUniformIntParam("texture", 0);

    double h, s, l;
    m_Color.toHSL(h, s, l);
    pShader->setUniformFloatParam("hKey", h);
    pShader->setUniformFloatParam("hTolerance", m_HTolerance*360);
    pShader->setUniformFloatParam("hSoftTolerance", (m_HTolerance+m_Softness)*360.0);
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

void GPUChromaKeyFilter::initShader()
{
    string sProgram =
        "uniform float alpha;\n"
        "uniform sampler2D texture;\n"
        "uniform float hKey;\n"
        "uniform float hTolerance;\n"
        "uniform float hSoftTolerance;\n"
        "uniform float sTolerance;\n"
        "uniform float sSoftTolerance;\n"
        "uniform float sKey;\n"
        "uniform float lTolerance;\n"
        "uniform float lSoftTolerance;\n"
        "uniform float spillThreshold;\n"
        "uniform float lKey;\n"
        "uniform bool bIsLast;\n"

        "void rgb2hsl(vec4 rgba, out float h, out float s, out float l)\n"
        "{\n"
        "    float maxComp = max(rgba.r, max(rgba.g, rgba.b));\n"
        "    float minComp = min(rgba.r, min(rgba.g, rgba.b));\n"
        "    l = (maxComp+minComp)/2.0;\n" 
        "    if (maxComp == minComp) {\n"
        "        s = 0.0;\n"
        "        h = 0.0;\n"
        "    } else {\n"
        "        float delta = maxComp-minComp;\n"
        "        if (l < 0.5) {\n"
        "            s = delta/(maxComp+minComp);\n"
        "        } else {\n"
        "            s = delta/(2.0-(maxComp+minComp));\n"
        "        }\n"
        "        if (rgba.r == maxComp) {\n"
        "            h = (rgba.g-rgba.b)/delta;\n"
        "            if (h < 0.0) {\n"
        "                h += 6.0;\n"
        "            }\n"
        "        } else if (rgba.g == maxComp) {\n"
        "            h = 2.0+(rgba.b-rgba.r)/delta;\n"
        "        } else {\n"
        "            h = 4.0+(rgba.r-rgba.g)/delta;\n"
        "        }\n"
        "        h *= 60.0;\n"
        "    }\n"
        "}\n"

        "vec3 hsl2rgb(float h, float s, float l)\n"
        "{\n"
        "    vec3 rgb = vec3(0.0, 0.0, 0.0);\n"
        "    float v;\n"
        "    if (l <= 0.5) {\n"
        "        v = l*(1.0+s);\n"
        "    } else {\n"
        "        v = l+s-l*s;\n"
        "    }\n"
        "    if (v > 0.0) {\n"
        "        float m = 2.0*l-v;\n"
        "        float sv = (v-m)/v;\n"
        "        h /= 60.0;\n"
        "        int sextant = int(h);\n"
        "        float fract = h-float(sextant);\n"
        "        float vsf = v * sv * fract;\n"
        "        float mid1 = m + vsf;\n"
        "        float mid2 = v - vsf;\n"
        "        if (sextant == 0) {\n"
        "            rgb.r = v;\n"
        "            rgb.g = mid1;\n"
        "            rgb.b = m;\n"
        "        } else if (sextant == 1) {\n"
        "            rgb.r = mid2;\n"
        "            rgb.g = v;\n"
        "            rgb.b = m;\n"
        "        } else if (sextant == 2) {\n"
        "            rgb.r = m;\n"
        "            rgb.g = v;\n"
        "            rgb.b = mid1;\n"
        "        } else if (sextant == 3) {\n"
        "            rgb.r = m;\n"
        "            rgb.g = mid2;\n"
        "            rgb.b = v;\n"
        "        } else if (sextant == 4) {\n"
        "            rgb.r = mid1;\n"
        "            rgb.g = m;\n"
        "            rgb.b = v;\n"
        "        } else if (sextant == 5) {\n"
        "            rgb.r = v;\n"
        "            rgb.g = m;\n"
        "            rgb.b = mid2;\n"
        "        }\n"
        "    }\n"
        "    return rgb;\n"
        "}\n"

        "void main(void)\n"
        "{\n"
        "    vec4 tex = texture2D(texture, gl_TexCoord[0].st);\n"
        "    float h;\n"
        "    float s;\n"
        "    float l;\n"
        "    float alpha;\n"
        "    rgb2hsl(tex, h, s, l);\n"
        "    float hDiff = abs(h-hKey);\n"
        "    float sDiff = abs(s-sKey);\n"
        "    float lDiff = abs(l-lKey);\n"
        "    if (hDiff < hSoftTolerance && sDiff < sSoftTolerance \n"
        "            && lDiff < lSoftTolerance)\n"
        "    {\n"
        "        alpha = 0.0;\n"
        "        if (hDiff > hTolerance) {\n"
        "            alpha = (hDiff-hTolerance)/(hSoftTolerance-hTolerance);\n"
        "        }\n"        
        "        if (sDiff > sTolerance) {\n"
        "            alpha = max(alpha,\n"
        "                   (sDiff-sTolerance)/(sSoftTolerance-sTolerance));\n"
        "        }\n"
        "        if (lDiff > lTolerance) {\n"
        "            alpha = max(alpha,\n"
        "                   (lDiff-lTolerance)/(lSoftTolerance-lTolerance));\n"
        "        }\n"
        "    } else {\n"
        "        alpha = 1.0;\n"
        "    }\n"
        "    if (hDiff < spillThreshold) {\n"
        "        float factor = 1.0-(spillThreshold-hDiff)/spillThreshold;\n"
        "        s = s*factor;\n"
/* Variant: Adjust hue        
        "        if (h < hKey) {\n"
        "            h = hKey-spillThreshold;\n"
        "        } else {\n"
        "            h = hKey+spillThreshold;\n"
        "        }\n"
*/        
        "        tex.rgb = hsl2rgb(h, s, l);\n"
        "    }\n"
        "    if (bIsLast) {\n"
        "       gl_FragColor = vec4(tex.rgb*alpha, alpha);\n"
        "    } else {\n"
        "       gl_FragColor = vec4(tex.rgb, alpha);\n"
        "    }\n"
        "}\n"
        ;
    getOrCreateShader(SHADERID_CHROMAKEY, sProgram);

    string sErosionProgram = 
        "uniform sampler2D texture;\n"
        "uniform bool bIsLast;\n"

        "void main(void)\n"
        "{\n"
        "    float minAlpha = 1.0;\n"
        "    float dx = dFdx(gl_TexCoord[0].x);\n"
        "    float dy = dFdy(gl_TexCoord[0].y);\n"
        "    for (float y = -1.0; y <= 1.0; ++y) {\n"
        "        for (float x = -1.0; x <= 1.0; ++x) {\n"
        "           float a = texture2D(texture, gl_TexCoord[0].st+vec2(x*dx,y*dy)).a;\n"
        "           minAlpha = min(minAlpha, a);\n"
        "        }\n"
        "    }\n"
        "    vec4 tex = texture2D(texture, gl_TexCoord[0].st);\n"
        "    if (bIsLast) {\n"
        "       gl_FragColor = vec4(tex.rgb*minAlpha, minAlpha);\n"
        "    } else {\n"
        "       gl_FragColor = vec4(tex.rgb, minAlpha);\n"
        "    }\n"
        "}\n"
        ;
    getOrCreateShader(SHADERID_EROSION, sErosionProgram);

}

}
