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

#include "GLTexture.h"

#include "../base/Exception.h"

namespace avg {

GLTexture::GLTexture(const IntPoint& size, PixelFormat pf, bool bMipmap,
        unsigned wrapSMode, unsigned wrapTMode)
    : m_Size(size),
      m_pf(pf),
      m_bMipmap(bMipmap)
{
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
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, getGLInternalFormat(), m_Size.x, m_Size.y, 0,
            getGLFormat(m_pf), getGLType(m_pf), 0);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "GLTexture: glTexImage2D()");
}

GLTexture::GLTexture(unsigned glTexID, const IntPoint& size, PixelFormat pf, bool bMipmap)
    : m_Size(size),
      m_pf(pf),
      m_bMipmap(bMipmap),
      m_TexID(glTexID)
{
}

GLTexture::~GLTexture()
{
    glBindTexture(GL_TEXTURE_2D, 0);
    glDeleteTextures(1, &m_TexID);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "GLTexture: DeleteTextures()");
}

void GLTexture::activate(int textureUnit)
{
    // TODO: This should set the complete texture state.
    glproc::ActiveTexture(textureUnit);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "GLTexture::activate ActiveTexture()");
    glBindTexture(GL_TEXTURE_2D, m_TexID);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "GLTexture::activate BindTexture()");
}

void GLTexture::generateMipmaps()
{
    activate();
    glproc::GenerateMipmap(GL_TEXTURE_2D);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "GLTexture::generateMipmaps()");
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

bool GLTexture::hasMipmaps() const
{
    return m_bMipmap;
}

}
