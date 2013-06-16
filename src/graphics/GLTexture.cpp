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
#include "../base/ObjectCounter.h"

#include "GLContext.h"
#include "TextureMover.h"
#ifndef AVG_ENABLE_EGL
    #include "PBO.h"
#endif
#include "FBO.h"

#include <string.h>
#include <iostream>

namespace avg {

using namespace std;

// We assign our own texture ids and never reuse them instead of using glGenTextures.
// That works very well, except that other components (e.g. Ogre3d) with shared gl 
// contexts don't know anything about our ids and thus use the same ones.
// Somewhat hackish solution: Assign ids starting with a very high id, so the id ranges
// don't overlap.
unsigned GLTexture::s_LastTexID = 10000000;

GLTexture::GLTexture(const IntPoint& size, PixelFormat pf, bool bMipmap,
        int potBorderColor, unsigned wrapSMode, unsigned wrapTMode, bool bForcePOT)
    : m_Size(size),
      m_pf(pf),
      m_bMipmap(bMipmap),
      m_bDeleteTex(true),
      m_bIsDirty(true)
{
    m_pGLContext = GLContext::getCurrent();
    ObjectCounter::get()->incRef(&typeid(*this));
    m_bUsePOT = m_pGLContext->usePOTTextures() || bForcePOT;
    if (m_pGLContext->isGLES() && bMipmap) {
        m_bUsePOT = true;
    }
    if (m_bUsePOT) {
        m_GLSize.x = nextpow2(m_Size.x);
        m_GLSize.y = nextpow2(m_Size.y);
    } else {
        m_GLSize = m_Size;
    }

    int maxTexSize = m_pGLContext->getMaxTexSize();
    if (m_Size.x > maxTexSize || m_Size.y > maxTexSize) {
        throw Exception(AVG_ERR_VIDEO_GENERAL, "Texture too large ("  + toString(m_Size)
                + "). Maximum supported by graphics card is "
                + toString(maxTexSize));
    }
    if (getGLType(m_pf) == GL_FLOAT && !isFloatFormatSupported()) {
        throw Exception(AVG_ERR_UNSUPPORTED, 
                "Float textures not supported by OpenGL configuration.");
    }

    s_LastTexID++;
    m_TexID = s_LastTexID;
    m_pGLContext->bindTexture(GL_TEXTURE0, m_TexID);
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
    GLContext::checkError("GLTexture: glTexImage2D()");

    if (m_bUsePOT) {
        // Make sure the texture is transparent and black before loading stuff 
        // into it to avoid garbage at the borders.
        // In the case of UV textures, we set the border color to 128...
        int TexMemNeeded = m_GLSize.x*m_GLSize.y*getBytesPerPixel(m_pf);
        char * pPixels = new char[TexMemNeeded];
        memset(pPixels, potBorderColor, TexMemNeeded);
        glTexImage2D(GL_TEXTURE_2D, 0, getGLInternalFormat(), m_GLSize.x, 
                m_GLSize.y, 0, getGLFormat(m_pf), getGLType(m_pf), 
                pPixels);
        GLContext::checkError("PBOTexture::createTexture: glTexImage2D()");
        delete[] pPixels;
    }
//    dump(wrapSMode, wrapTMode);
}

GLTexture::GLTexture(unsigned glTexID, const IntPoint& size, PixelFormat pf, bool bMipmap,
        bool bDeleteTex)
    : m_Size(size),
      m_GLSize(size),
      m_pf(pf),
      m_bMipmap(bMipmap),
      m_bDeleteTex(bDeleteTex),
      m_bUsePOT(false),
      m_TexID(glTexID),
      m_bIsDirty(true)
{
    m_pGLContext = GLContext::getCurrent();
    ObjectCounter::get()->incRef(&typeid(*this));
}

GLTexture::~GLTexture()
{
    if (m_bDeleteTex) {
        glDeleteTextures(1, &m_TexID);
        GLContext::checkError("GLTexture: DeleteTextures()");
    }
    ObjectCounter::get()->decRef(&typeid(*this));
}

void GLTexture::activate(int textureUnit)
{
    m_pGLContext->bindTexture(textureUnit, m_TexID);
}

void GLTexture::generateMipmaps()
{
    if (m_bMipmap) {
        activate();
        glproc::GenerateMipmap(GL_TEXTURE_2D);
        GLContext::checkError("GLTexture::generateMipmaps()");
    }
}

void GLTexture::setWrapMode(unsigned wrapSMode, unsigned wrapTMode)
{
    activate();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapSMode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapTMode);
}

void GLTexture::enableStreaming()
{
    m_pMover = TextureMover::create(m_Size, m_pf, GL_STREAM_DRAW);
}

BitmapPtr GLTexture::lockStreamingBmp()
{
    AVG_ASSERT(m_pMover);
    return m_pMover->lock();
}

void GLTexture::unlockStreamingBmp(bool bUpdated)
{
    AVG_ASSERT(m_pMover);
    m_pMover->unlock();
    if (bUpdated) {
        m_pMover->moveToTexture(*this);
        m_bIsDirty = true;
    }
}

void GLTexture::moveBmpToTexture(BitmapPtr pBmp)
{
    TextureMoverPtr pMover = TextureMover::create(m_Size, m_pf, GL_DYNAMIC_DRAW);
    pMover->moveBmpToTexture(pBmp, *this);
    m_bIsDirty = true;
}

BitmapPtr GLTexture::moveTextureToBmp(int mipmapLevel)
{
    TextureMoverPtr pMover = TextureMover::create(m_GLSize, m_pf, GL_DYNAMIC_READ);
    return pMover->moveTextureToBmp(*this, mipmapLevel);
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

IntPoint GLTexture::getMipmapSize(int level) const
{
    IntPoint size = m_Size;
    for (int i=0; i<level; ++i) {
        size.x = max(1, size.x >> 1);
        size.y = max(1, size.y >> 1);
    }
    return size;
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
            AVG_ASSERT(!GLContext::getMain()->isGLES());
            return GL_BGRA;
#ifndef AVG_ENABLE_EGL            
        case R32G32B32A32F:
            return GL_BGRA;
#endif
        case R8G8B8:
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
        case R8G8B8:
            return GL_UNSIGNED_BYTE;
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
        case A8:
            return GL_ALPHA;
        case R8G8B8A8:
        case R8G8B8X8:
            return GL_RGBA;
        case B8G8R8A8:
        case B8G8R8X8:
            AVG_ASSERT(!GLContext::getMain()->isGLES());
            return GL_RGBA;
#ifndef AVG_ENABLE_EGL            
        case R32G32B32A32F:
            return GL_RGBA32F_ARB;
        case I32F:
            return GL_LUMINANCE32F_ARB;
#endif
        case R8G8B8:
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

void GLTexture::dump(unsigned wrapSMode, unsigned wrapTMode) const
{
    cerr << "GLTexture" << endl;
    cerr << "m_Size: " << m_Size << endl;
    cerr << "m_GLSize: " << m_Size << endl;
    cerr << "m_pf: " << m_pf << endl;
    cerr << "m_bMipmap: " << m_bMipmap << endl;
    if (wrapSMode != (unsigned)-1) {
        cerr << "Wrap modes: " << wrapSMode << ", " << wrapTMode << endl;
    }
}

}
