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

#include "GLTexture.h"

#include "../base/Exception.h"
#include "../base/StringHelper.h"
#include "../base/MathHelper.h"

#include "GLContext.h"

#include <string.h>
#include <iostream>

namespace avg {

using namespace std;

GLTexture::GLTexture(const IntPoint& size, PixelFormat pf, bool bMipmap,
        unsigned wrapSMode, unsigned wrapTMode, bool bForcePOT)
    : m_Size(size),
      m_pf(pf),
      m_bMipmap(bMipmap),
      m_bDeleteTex(true)
{
    bool bUsePOT = GLContext::getCurrent()->usePOTTextures() || bForcePOT;
    if (bUsePOT) {
        m_GLSize.x = nextpow2(m_Size.x);
        m_GLSize.y = nextpow2(m_Size.y);
    } else {
        m_GLSize = m_Size;
    }

    int maxTexSize = GLContext::getCurrent()->getMaxTexSize();
    if (m_Size.x > maxTexSize || m_Size.y > maxTexSize) {
        throw Exception(AVG_ERR_VIDEO_GENERAL, "Texture too large ("  + toString(m_Size)
                + "). Maximum supported by graphics card is "
                + toString(maxTexSize));
    }
    if (getGLType(m_pf) == GL_FLOAT && !isFloatFormatSupported()) {
        throw Exception(AVG_ERR_UNSUPPORTED, 
                "Float textures not supported by OpenGL configuration.");
    }

    glGenTextures(1, &m_TexID);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "GLTexture: glGenTextures()");
    glBindTexture(GL_TEXTURE_2D, m_TexID);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "GLTexture: glBindTexture()");
    if (bMipmap) {
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    } else {
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    }
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapSMode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapTMode);
    glTexImage2D(GL_TEXTURE_2D, 0, getGLInternalFormat(), m_GLSize.x, m_GLSize.y, 0,
            getGLFormat(m_pf), getGLType(m_pf), 0);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "GLTexture: glTexImage2D()");

    if (bUsePOT) {
        // Make sure the texture is transparent and black before loading stuff 
        // into it to avoid garbage at the borders.
        int TexMemNeeded = m_GLSize.x*m_GLSize.y*Bitmap::getBytesPerPixel(m_pf);
        char * pPixels = new char[TexMemNeeded];
        memset(pPixels, 0, TexMemNeeded);
        glTexImage2D(GL_TEXTURE_2D, 0, getGLInternalFormat(), m_GLSize.x, 
                m_GLSize.y, 0, getGLFormat(m_pf), getGLType(m_pf), 
                pPixels);
        OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "PBOTexture::createTexture: glTexImage2D()");
        delete[] pPixels;
    }
}

GLTexture::GLTexture(unsigned glTexID, const IntPoint& size, PixelFormat pf, bool bMipmap,
        bool bDeleteTex)
    : m_Size(size),
      m_GLSize(size),
      m_pf(pf),
      m_bMipmap(bMipmap),
      m_bDeleteTex(bDeleteTex),
      m_TexID(glTexID)
{
}

GLTexture::~GLTexture()
{
    if (m_bDeleteTex) {
        glBindTexture(GL_TEXTURE_2D, 0);
        glDeleteTextures(1, &m_TexID);
        OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "GLTexture: DeleteTextures()");
    }
}

void GLTexture::activate(int textureUnit)
{
    glproc::ActiveTexture(textureUnit);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "GLTexture::activate ActiveTexture()");
    glBindTexture(GL_TEXTURE_2D, m_TexID);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "GLTexture::activate BindTexture()");
}

void GLTexture::generateMipmaps()
{
    if (m_bMipmap) {
        activate();
        glproc::GenerateMipmap(GL_TEXTURE_2D);
        OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "GLTexture::generateMipmaps()");
    }
}

void GLTexture::setWrapMode(unsigned wrapSMode, unsigned wrapTMode)
{
    activate();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapSMode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapTMode);
}

const IntPoint& GLTexture::getSize() const
{
    return m_Size;
}

const IntPoint& GLTexture::getGLSize() const
{
    return m_GLSize;
}

PixelFormat GLTexture::getPF() const
{
    return m_pf;
}

unsigned GLTexture::getID() const
{
    return m_TexID;
}

bool GLTexture::isFloatFormatSupported()
{
    return queryOGLExtension("GL_ARB_texture_float");
}

int GLTexture::getGLFormat(PixelFormat pf)
{
    switch (pf) {
        case I8:
        case I32F:
            return GL_LUMINANCE;
        case A8:
            return GL_ALPHA;
        case R8G8B8A8:
        case R8G8B8X8:
            return GL_RGBA;
        case B8G8R8A8:
        case B8G8R8X8:
        case R32G32B32A32F:
            return GL_BGRA;
        case B5G6R5:
            return GL_RGB;
        default:
            AVG_ASSERT(false);
            return 0;
    }
}

int GLTexture::getGLType(PixelFormat pf)
{
    switch (pf) {
        case I8:
        case A8:
            return GL_UNSIGNED_BYTE;
        case R8G8B8A8:
        case R8G8B8X8:
        case B8G8R8A8:
        case B8G8R8X8:
#ifdef __APPLE__
            return GL_UNSIGNED_INT_8_8_8_8_REV;
#else
            return GL_UNSIGNED_BYTE;
#endif
        case R32G32B32A32F:
        case I32F:
            return GL_FLOAT;
        case B5G6R5:
            return GL_UNSIGNED_SHORT_5_6_5;
        default:
            AVG_ASSERT(false);
            return 0;
    }
}

int GLTexture::getGLInternalFormat() const
{
    switch (m_pf) {
        case I8:
            return GL_LUMINANCE;
        case I32F:
            return GL_LUMINANCE32F_ARB;
        case A8:
            return GL_ALPHA;
        case R8G8B8A8:
        case R8G8B8X8:
        case B8G8R8A8:
        case B8G8R8X8:
            return GL_RGBA;
        case R32G32B32A32F:
            return GL_RGBA32F_ARB;
        case B5G6R5:
            return GL_RGB;
        default:
            AVG_ASSERT(false);
            return 0;
    }
}

void GLTexture::setDirty()
{
    m_bIsDirty = true;
}

bool GLTexture::isDirty() const
{
    return m_bIsDirty;
}

void GLTexture::resetDirty()
{
    m_bIsDirty = false;
}


}
