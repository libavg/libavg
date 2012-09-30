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

#include "GPUInvertFilter.h"
#include "ShaderRegistry.h"
#include "OGLShader.h"

#include "../base/ObjectCounter.h"
#include "../base/Logger.h"

#define SHADERID_INVERT_COLOR "invert"

using namespace std;

namespace avg {

GPUInvertFilter::GPUInvertFilter(const IntPoint& size, PixelFormat pf, bool bStandalone)
    : GPUFilter(pf, B8G8R8A8, bStandalone, SHADERID_INVERT_COLOR, 1)
{
    ObjectCounter::get()->incRef(&typeid(*this));
    setDimensions(size);
    m_pTextureParam = getShader()->getParam<int>("u_Texture");
}

GPUInvertFilter::~GPUInvertFilter()
{
    ObjectCounter::get()->decRef(&typeid(*this));
}

void GPUInvertFilter::applyOnGPU(GLTexturePtr pSrcTex)
{
    glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);
    getShader()->activate();
    m_pTextureParam->set(0);
    draw(pSrcTex);
}

}

