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

#define SHADERID "CHROMAKEY"

using namespace std;

namespace avg {

GPUChromaKeyFilter::GPUChromaKeyFilter(const IntPoint& size, PixelFormat pf, 
        bool bStandalone)
    : GPUFilter(size, pf, B8G8R8A8, bStandalone),
      m_Color(0, 255, 0),
      m_HTolerance(30),
      m_STolerance(0.1),
      m_LTolerance(0.4),
      m_Softness(0.0)
{
    ObjectCounter::get()->incRef(&typeid(*this));

    initShader();
}

GPUChromaKeyFilter::~GPUChromaKeyFilter()
{
    ObjectCounter::get()->decRef(&typeid(*this));
}

void GPUChromaKeyFilter::setColor(const Pixel32& color)
{
    m_Color = color;
}

void GPUChromaKeyFilter::setHueTolerance(double tolerance)
{
    m_HTolerance = tolerance;
}

void GPUChromaKeyFilter::setSaturationTolerance(double tolerance)
{
    m_STolerance = tolerance;
}

void GPUChromaKeyFilter::setLightnessTolerance(double tolerance)
{
    m_LTolerance = tolerance;
}

void GPUChromaKeyFilter::setSoftness(double softness)
{
    m_Softness = softness;
}


void GPUChromaKeyFilter::applyOnGPU(GLTexturePtr pSrcTex)
{
    OGLShaderPtr pShader = getShader(SHADERID);
    pShader->activate();
    pShader->setUniformIntParam("texture", 0);

    double h, s, l;
    m_Color.toHSL(h, s, l);
    pShader->setUniformFloatParam("hKey", h);
    pShader->setUniformFloatParam("hTolerance", m_HTolerance);
    pShader->setUniformFloatParam("hSoftTolerance", m_HTolerance+m_Softness*360.0);
    pShader->setUniformFloatParam("sKey", s);
    pShader->setUniformFloatParam("sTolerance", m_STolerance);
    pShader->setUniformFloatParam("sSoftTolerance", m_STolerance+m_Softness);
    pShader->setUniformFloatParam("lKey", l);
    pShader->setUniformFloatParam("lTolerance", m_LTolerance);
    pShader->setUniformFloatParam("lSoftTolerance", m_LTolerance+m_Softness);
    draw(pSrcTex);

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
        "uniform float lKey;\n"

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

        "bool inBetween(float val, float min, float max)\n"
        "{\n"
        "    return (val >= min && val <= max);\n"
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
        "    gl_FragColor.rgba = vec4(tex.rgb*alpha, alpha);\n"
        "}\n"
        ;

    getOrCreateShader(SHADERID, sProgram);
}

}
