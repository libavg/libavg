//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2020 Ulrich von Zadow
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
#include "GLContextManager.h"
#include "TextureMover.h"

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

GLTexture::GLTexture(GLContext* pContext, const TexInfo& texInfo)
    : TexInfo(texInfo),
      m_pContext(pContext)      
{
    ObjectCounter::get()->incRef(&typeid(*this));
    init();

}

GLTexture::GLTexture(GLContext* pContext, const IntPoint& size, PixelFormat pf,
        bool bMipmap, bool bForcePOT, int potBorderColor)
    : TexInfo(size, pf, bMipmap, usePOT(bForcePOT, bMipmap), potBorderColor),
      m_pContext(pContext)
{
    ObjectCounter::get()->incRef(&typeid(*this));
    init();
}

GLTexture::~GLTexture()
{
    if (GLContextManager::isActive()) {
        GLContextManager::get()->deleteTexture(m_TexID);
    }
    ObjectCounter::get()->decRef(&typeid(*this));
}

void GLTexture::init()
{
    s_LastTexID++;
    m_TexID = s_LastTexID;

    m_pContext->bindTexture(GL_TEXTURE0, m_TexID);

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    IntPoint size = getGLSize();
    PixelFormat pf = getPF();
    glTexImage2D(GL_TEXTURE_2D, 0, getGLInternalFormat(), size.x, size.y, 0,
            getGLFormat(pf), getGLType(pf), 0);
    GLContext::checkError("GLTexture: glTexImage2D()");
    if (getUseMipmap()) {
        glproc::GenerateMipmap(GL_TEXTURE_2D);
        GLContext::checkError("GLTexture::GLTexture generateMipmap()");
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    } else {
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    }
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, m_WrapMode.getS());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, m_WrapMode.getT());

    if (getUsePOT()) {
        // Make sure the texture is transparent and black before loading stuff 
        // into it to avoid garbage at the borders.
        // In the case of UV textures, we set the border color to 128...
        int texMemNeeded = size.x*size.y*getBytesPerPixel(pf);
        char * pPixels = new char[texMemNeeded];
        memset(pPixels, getPOTBorderColor(), texMemNeeded);
        glTexImage2D(GL_TEXTURE_2D, 0, getGLInternalFormat(), size.x, size.y, 0, 
                getGLFormat(pf), getGLType(pf), pPixels);
        GLContext::checkError("GLTexture::init: glTexImage2D()");
        delete[] pPixels;
    }
}

void GLTexture::activate(const WrapMode& wrapMode, int textureUnit)
{
    m_pContext->bindTexture(textureUnit, m_TexID);
    if (wrapMode.getS() != m_WrapMode.getS() || wrapMode.getT() != m_WrapMode.getT()) {
        glproc::ActiveTexture(textureUnit);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode.getS());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode.getT());
        m_WrapMode = wrapMode;
    }
}

void GLTexture::generateMipmaps()
{
    if (getUseMipmap()) {
        m_pContext->bindTexture(GL_TEXTURE0, m_TexID);
        glproc::GenerateMipmap(GL_TEXTURE_2D);
        GLContext::checkError("GLTexture::generateMipmap()");
    }
}

void GLTexture::moveBmpToTexture(BitmapPtr pBmp)
{
    unsigned usage = GL_DYNAMIC_DRAW;
    if (getPF() == A8 && m_pContext->isVendor("ATI")) {
        // Workaround for https://github.com/libavg/libavg/issues/687
        usage = GL_STATIC_DRAW;
    }
    TextureMoverPtr pMover = TextureMover::create(getSize(), getPF(), usage);
    pMover->moveBmpToTexture(pBmp, *this);
}

BitmapPtr GLTexture::moveTextureToBmp(int mipmapLevel)
{
    TextureMoverPtr pMover = TextureMover::create(getGLSize(), getPF(), GL_DYNAMIC_READ);
    return pMover->moveTextureToBmp(*this, mipmapLevel);
}

unsigned GLTexture::getID() const
{
    return m_TexID;
}

}
