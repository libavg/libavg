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

#include "PBOTexture.h"

#include "../graphics/GLContext.h"
#include "../base/Logger.h"
#include "../base/Exception.h"
#include "../base/ScopeTimer.h"
#include "../base/ObjectCounter.h"
#include "../base/StringHelper.h"

#include <iostream>
#include <string>

namespace avg {

using namespace std;
    
PBOTexture::PBOTexture(IntPoint size, PixelFormat pf, const MaterialInfo& material) 
    : m_pf(pf),
      m_Material(material)
{
    ObjectCounter::get()->incRef(&typeid(*this));
    m_MemoryMode = GLContext::getCurrent()->getMemoryModeSupported();
    m_pTex = GLTexturePtr(new GLTexture(size, m_pf, m_Material.getUseMipmaps(),
            m_Material.getTexWrapSMode(), m_Material.getTexWrapTMode())); 
    createBitmap();
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
            m_pReadPBO = PBOPtr(new PBO(m_pTex->getGLSize(), m_pf, GL_DYNAMIC_READ));
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
    } else {
        m_pTex->activate();
        unsigned char * pStartPos = m_pBmp->getPixels();
        IntPoint size = m_pTex->getSize();
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, size.x, size.y,
                m_pTex->getGLFormat(m_pf), m_pTex->getGLType(m_pf), 
                pStartPos);
        OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "PBOTexture::download: glTexSubImage2D()");
    }
    m_pTex->generateMipmaps();
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

const IntPoint& PBOTexture::getTextureSize() const
{
    return m_pTex->getGLSize();
}

void PBOTexture::createBitmap()
{
    IntPoint size = m_pTex->getSize();
    switch (m_MemoryMode) {
        case MM_PBO:
            {
                m_pWritePBO = PBOPtr(new PBO(size, m_pf, GL_DYNAMIC_DRAW));
                m_pBmp = BitmapPtr();
            }
            break;
        case MM_OGL:
            m_pBmp = BitmapPtr(new Bitmap(size, m_pf));
            break;
        default:
            AVG_ASSERT(0);
    }
}


}
