//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2014 Ulrich von Zadow
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

#include "FBOInfo.h"

#include "OGLHelper.h"
#include "GLContext.h"

#include "../base/Exception.h"
#include "../base/StringHelper.h"

#include <stdio.h>

using namespace std;
using namespace boost;

namespace avg {

FBOInfo::FBOInfo(const IntPoint& size, PixelFormat pf, unsigned numTextures, 
        unsigned multisampleSamples, bool bUsePackedDepthStencil, bool bUseStencil,
        bool bMipmap)
    : m_Size(size),
      m_PF(pf),
      m_MultisampleSamples(multisampleSamples),
      m_bUsePackedDepthStencil(bUsePackedDepthStencil),
      m_bUseStencil(bUseStencil),
      m_bMipmap(bMipmap)
{
    AVG_ASSERT(numTextures == 1 || multisampleSamples == 1);
    if (multisampleSamples > 1 && !(isMultisampleFBOSupported())) {
        throw Exception(AVG_ERR_UNSUPPORTED, 
                "Multisample offscreen rendering is not supported by this OpenGL driver/card combination.");
    }
    if (multisampleSamples < 1) {
        throwMultisampleError();
    }
}

FBOInfo::~FBOInfo()
{
}

const IntPoint& FBOInfo::getSize() const
{
    return m_Size;
}

bool FBOInfo::isFBOSupported()
{
    return GLContext::getCurrent()->isGLES() || 
            queryOGLExtension("GL_EXT_framebuffer_object");
}

bool FBOInfo::isMultisampleFBOSupported()
{
#ifdef AVG_ENABLE_EGL
    return false;
#else
    int maxSamples;
    glGetIntegerv(GL_MAX_SAMPLES_EXT, &maxSamples);
    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        return false;
    }
    return queryOGLExtension("GL_EXT_framebuffer_multisample") && 
            queryOGLExtension("GL_EXT_framebuffer_blit") && maxSamples > 1;
#endif
}

bool FBOInfo::isPackedDepthStencilSupported()
{
    return queryOGLExtension("GL_EXT_packed_depth_stencil") || 
            queryOGLExtension("GL_OES_packed_depth_stencil");
}

PixelFormat FBOInfo::getPF() const
{
    return m_PF;
}

unsigned FBOInfo::getMultisampleSamples() const
{
    return m_MultisampleSamples;
}

bool FBOInfo::getUsePackedDepthStencil() const
{
    return m_bUsePackedDepthStencil;
}

bool FBOInfo::getUseStencil() const
{
    return m_bUseStencil;
}

bool FBOInfo::getMipmap() const
{
    return m_bMipmap;
}

void FBOInfo::throwMultisampleError()
{
    throw(Exception(AVG_ERR_UNSUPPORTED, 
                string("Unsupported value for number of multisample samples (")
                + toString(m_MultisampleSamples) + ")."));
}

}


