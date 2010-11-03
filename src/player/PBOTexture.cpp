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

#include "PBOTexture.h"
#include "SDLDisplayEngine.h"
#include "../graphics/VertexArray.h"
#include "../base/Logger.h"
#include "../base/Exception.h"
#include "../base/ScopeTimer.h"
#include "../base/ObjectCounter.h"
#include "../base/MathHelper.h"
#include "../base/StringHelper.h"

#include <iostream>
#include <string>

namespace avg {

using namespace std;
    
PBOTexture::PBOTexture(IntPoint size, PixelFormat pf, const MaterialInfo& material,
        SDLDisplayEngine * pEngine, OGLMemoryMode memoryMode) 
    : m_pf(pf),
      m_Material(material),
      m_pEngine(pEngine),
      m_MemoryMode(memoryMode)
{
    ObjectCounter::get()->incRef(&typeid(*this));
    m_ActiveSize = size;
    if (pEngine->usePOTTextures()) {
        m_Size.x = nextpow2(m_ActiveSize.x);
        m_Size.y = nextpow2(m_ActiveSize.y);
    } else {
        m_Size = m_ActiveSize;
    }
    if (m_Size.x > pEngine->getMaxTexSize() || m_Size.y > pEngine->getMaxTexSize()) {
        throw Exception(AVG_ERR_VIDEO_GENERAL, "Texture too large (" +toString(m_Size)
                + "). Maximum supported by graphics card is "
                + toString(pEngine->getMaxTexSize()));
    }
    createBitmap();
    createTexture();
}

PBOTexture::~PBOTexture()
{
    ObjectCounter::get()->decRef(&typeid(*this));
}

BitmapPtr PBOTexture::lockBmp()
{
    if (m_MemoryMode == MM_PBO) {
        return m_pWritePBO->lock();
    } else {
        return m_pBmp;
    }
}

void PBOTexture::unlockBmp()
{
    if (m_MemoryMode == MM_PBO) {
        m_pWritePBO->unlock();
    }
}

BitmapPtr PBOTexture::readbackBmp()
{
    if (m_MemoryMode == MM_PBO) {
        if (!m_pReadPBO) {
            m_pReadPBO = PBOPtr(new PBO(m_Size, m_pf, GL_DYNAMIC_READ));
        }
        return m_pReadPBO->moveTextureToBmp(m_pTex);
    } else {
        return BitmapPtr(new Bitmap(*m_pBmp));
    }
}

static ProfilingZoneID TexSubImageProfilingZone("Texture download");

void PBOTexture::download() const
{
    ScopeTimer Timer(TexSubImageProfilingZone);
    if (m_MemoryMode == MM_PBO) {
        m_pWritePBO->movePBOToTexture(m_pTex);
        if (m_pTex->hasMipmaps()) {
            m_pTex->generateMipmaps();
        }
    } else {
        m_pTex->activate();
        glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "PBOTexture::download: GL_UNPACK_ALIGNMENT");
        unsigned char * pStartPos = m_pBmp->getPixels();
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_ActiveSize.x, m_ActiveSize.y,
                m_pTex->getGLFormat(m_pf), m_pTex->getGLType(m_pf), 
                pStartPos);
        OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "PBOTexture::download: glTexSubImage2D()");
    }
}

void PBOTexture::setTex(GLTexturePtr pTex)
{
    AVG_ASSERT(m_MemoryMode == MM_PBO);
    AVG_ASSERT(!m_pBmp);

    m_pTex = pTex;
}

void PBOTexture::activate(int textureUnit)
{
    m_pTex->activate(textureUnit);
}

void PBOTexture::setMaterial(const MaterialInfo& material)
{
    if (m_Material.getUseMipmaps() != material.getUseMipmaps()) {
        m_Material = material;
        createTexture();
    } else {
        m_Material = material;
    }
}

const IntPoint& PBOTexture::getTextureSize() const
{
    return m_Size;
}

void PBOTexture::createBitmap()
{
    switch (m_MemoryMode) {
        case MM_PBO:
            {
                m_pWritePBO = PBOPtr(new PBO(m_ActiveSize, m_pf, GL_DYNAMIC_DRAW));
                m_pBmp = BitmapPtr();
            }
            break;
        case MM_OGL:
            m_pBmp = BitmapPtr(new Bitmap(m_ActiveSize, m_pf));
            break;
        default:
            AVG_ASSERT(0);
    }
}

void PBOTexture::createTexture()
{
    m_pTex = GLTexturePtr(new GLTexture(m_Size, m_pf, m_Material.getUseMipmaps(),
            m_Material.getTexWrapSMode(), m_Material.getTexWrapTMode())); 

    if (m_pEngine->usePOTTextures()) {
        // Make sure the texture is transparent and black before loading stuff 
        // into it to avoid garbage at the borders.
        int TexMemNeeded = m_Size.x*m_Size.y*Bitmap::getBytesPerPixel(m_pf);
        char * pPixels = new char[TexMemNeeded];
        memset(pPixels, 0, TexMemNeeded);
        glTexImage2D(GL_TEXTURE_2D, 0, m_pTex->getGLInternalFormat(), m_Size.x, 
                m_Size.y, 0, m_pTex->getGLFormat(m_pf), m_pTex->getGLType(m_pf), pPixels);
        OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "PBOTexture::createTexture: glTexImage2D()");
        free(pPixels);
    }
}

}
