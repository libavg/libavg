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

#include "GPURGB2YUVFilter.h"
#include "Bitmap.h"
#include "ShaderRegistry.h"
#include "ImagingProjection.h"

#include "../base/ObjectCounter.h"
#include "../base/MathHelper.h"
#include "../base/Exception.h"

#include <string.h>
#include <iostream>

#define SHADERID "RGB2YUV"

using namespace std;

namespace avg {

GPURGB2YUVFilter::GPURGB2YUVFilter(const IntPoint& size)
    : GPUFilter(B8G8R8X8, B8G8R8X8, false)
{
    ObjectCounter::get()->incRef(&typeid(*this));

    setDimensions(size);
    initShader();
}

GPURGB2YUVFilter::~GPURGB2YUVFilter()
{
    ObjectCounter::get()->decRef(&typeid(*this));
}

void GPURGB2YUVFilter::applyOnGPU(GLTexturePtr pSrcTex)
{
    OGLShaderPtr pShader = getShader(SHADERID);
    pShader->activate();
    draw(pSrcTex);
    glproc::UseProgramObject(0);
}

BitmapPtr GPURGB2YUVFilter::getResults()
{
    BitmapPtr pBmp = getFBO()->getImage();

    return pBmp;
}

void GPURGB2YUVFilter::initShader()
{
    // Uses jpeg coefficients.
    string sProgram = 
        "uniform sampler2D texture;\n"
        "\n"
        "void main(void)\n"
        "{\n"
        "    vec4 tex = texture2D(texture, gl_TexCoord[0].st);\n"
        "    float y =  0.299*tex.r + 0.587*tex.g + 0.114*tex.b;\n"
        "    float u = -0.168*tex.r - 0.330*tex.g + 0.498*tex.b + 0.5;\n"
        "    float v =  0.498*tex.r - 0.417*tex.g - 0.081*tex.b + 0.5;\n"
        "    gl_FragColor = vec4(v,u,y,1);\n"
        "}\n"
        ;
    getOrCreateShader(SHADERID, sProgram);
}

}
