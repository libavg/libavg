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

    string sProgramHead = 
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
        + getStdShaderCode()
        ;
        
    string sProgram = sProgramHead +
        "vec4 alphaMin(vec4 v1, vec4 v2)\n"
        "{\n"
        "    if (v1.a < v2.a) {\n"
        "        return v1;\n"
        "    } else {\n"
        "        return v2;\n"
        "    }\n"
        "}\n"
    
        "vec4 alphaMax(vec4 v1, vec4 v2)\n"
        "{\n"
        "    if (v1.a < v2.a) {\n"
        "        return v2;\n"
        "    } else {\n"
        "        return v1;\n"
        "    }\n"
        "}\n"

        "#define s2(a, b)    temp = a; a = alphaMin(a, b); b = alphaMax(temp, b);\n"
        "#define mn3(a, b, c)            s2(a, b); s2(a, c);\n"
        "#define mx3(a, b, c)            s2(b, c); s2(a, c);\n"

        "#define mnmx3(a, b, c)          mx3(a, b, c); s2(a, b);\n"
        "#define mnmx4(a, b, c, d)       s2(a, b); s2(c, d); s2(a, c); s2(b, d); \n"

        "// Based on McGuire, A fast, small-radius GPU median filter, in ShaderX6,\n"
        "// February 2008. http://graphics.cs.williams.edu/papers/MedianShaderX6/ \n"
        "vec4 getMedian(vec2 texCoord)\n"
        "{\n"
        "    vec4 v[5];\n"
        "    float dx = dFdx(texCoord.x);\n"
        "    float dy = dFdy(texCoord.y);\n"
        "    v[0] = texture2D(texture, texCoord);\n"
        "    v[1] = texture2D(texture, texCoord+vec2(0,-dy));\n"
        "    v[2] = texture2D(texture, texCoord+vec2(0,dy));\n"
        "    v[3] = texture2D(texture, texCoord+vec2(-dx,0));\n"
        "    v[4] = texture2D(texture, texCoord+vec2(dx,0));\n"
        "    for (int i = 0; i < 5; ++i) {\n"
        "        v[i].a = (v[i].r+v[i].g+v[i].b)/3.0;\n"
        "    }\n"

        "    vec4 temp;\n"
        "    mnmx4(v[0], v[1], v[2], v[3]);\n"
        "    mnmx3(v[1], v[2], v[4]);\n"
        "    return v[2];\n"
        "}\n"

        "void main(void)\n"
        "{\n"
        "    vec4 tex = getMedian(gl_TexCoord[0].st);\n"
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
        "    if (alpha > 0.0 && hDiff < spillThreshold) {\n"
        "        if (spillThreshold > hTolerance) {\n"
        "            float factor = max(0.0, 1.0-(spillThreshold-hDiff)\n"
        "                    /(spillThreshold-hTolerance));\n"
        "            s = s*factor;\n"
        "        }\n"
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
