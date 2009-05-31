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

#include "OGLSurface.h"
#include "Player.h"
#include "SDLDisplayEngine.h"

#include "../base/MathHelper.h"
#include "../base/Logger.h"
#include "../base/Exception.h"
#include "../base/ScopeTimer.h"
#include "../base/ObjectCounter.h"

#include <iostream>
#include <sstream>

using namespace std;

namespace avg {

OGLSurface::OGLSurface(const MaterialInfo& material)
    : m_Size(-1,-1),
      m_Material(material),
      m_pEngine(0)
{
    ObjectCounter::get()->incRef(&typeid(*this));
}

OGLSurface::~OGLSurface()
{
    ObjectCounter::get()->decRef(&typeid(*this));
}

void OGLSurface::attach(SDLDisplayEngine * pEngine)
{
    m_pEngine = pEngine;
    m_MemoryMode = m_pEngine->getMemoryModeSupported();
}

void OGLSurface::create(const IntPoint& size, PixelFormat pf)
{
    assert(m_pEngine);
    if (m_pTextures[0] && m_Size == size && m_pf == pf) {
        // If nothing's changed, we can ignore everything.
        return;
    }
    m_Size = size;
    m_pf = pf;

    if (m_pf == YCbCr420p || m_pf == YCbCrJ420p) {
        m_pTextures[0] = OGLTexturePtr(new OGLTexture(size, I8, m_Material, m_pEngine,
                m_MemoryMode));
        IntPoint halfSize(size.x/2, size.y/2);
        m_pTextures[1] = OGLTexturePtr(new OGLTexture(halfSize, I8, m_Material, m_pEngine,
                m_MemoryMode));
        m_pTextures[2] = OGLTexturePtr(new OGLTexture(halfSize, I8, m_Material, m_pEngine,
                m_MemoryMode));
    } else {
        m_pTextures[0] = OGLTexturePtr(new OGLTexture(size, m_pf, m_Material, m_pEngine,
                m_MemoryMode));
    }
}

void OGLSurface::createMask(const IntPoint& size)
{
    assert(m_pEngine);
    assert(m_Material.m_bHasMask);
    m_MaskSize = size;
    m_pMaskTexture = OGLTexturePtr(new OGLTexture(size, I8, m_Material, m_pEngine,
            m_MemoryMode));
}

void OGLSurface::destroy()
{
    m_pTextures[0] = OGLTexturePtr();
    m_pTextures[1] = OGLTexturePtr();
    m_pTextures[2] = OGLTexturePtr();
}

void OGLSurface::activate() const
{
    if (useShader()) {
        OGLShaderPtr pShader = m_pEngine->getShader();
        pShader->activate();
        OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "OGLSurface::activate()");
        switch (m_pf) {
            case YCbCr420p:
                pShader->setUniformIntParam("colorModel", 1);
                break;
            case YCbCrJ420p:
                pShader->setUniformIntParam("colorModel", 2);
                break;
            default:
                pShader->setUniformIntParam("colorModel", 0);
        }

        glproc::ActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_pTextures[0]->getTexID());
        pShader->setUniformIntParam("texture", 0);
        
        if (m_pf == YCbCr420p || m_pf == YCbCrJ420p) {
            glproc::ActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, m_pTextures[1]->getTexID());
            pShader->setUniformIntParam("cbTexture", 1);
            glproc::ActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, m_pTextures[2]->getTexID());
            pShader->setUniformIntParam("crTexture", 2);
        }
        
        pShader->setUniformIntParam("bUseMask", m_Material.m_bHasMask);
        if (m_Material.m_bHasMask) {
            glproc::ActiveTexture(GL_TEXTURE3);
            glBindTexture(GL_TEXTURE_2D, m_pMaskTexture->getTexID());
            pShader->setUniformIntParam("maskTexture", 3);
        }

        OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "OGLSurface::activate: params");
    } else {
        glproc::ActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_pTextures[0]->getTexID());
        if (m_pEngine->isUsingShaders()) {
            glproc::UseProgramObject(0);
        }
    }
}

void OGLSurface::deactivate() const
{
    if (m_pf == YCbCr420p || m_pf == YCbCrJ420p) {
        glproc::ActiveTexture(GL_TEXTURE1);
        glDisable(GL_TEXTURE_2D);
        glproc::ActiveTexture(GL_TEXTURE2);
        glDisable(GL_TEXTURE_2D);
        glproc::ActiveTexture(GL_TEXTURE0);
        glproc::UseProgramObject(0);
        OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "OGLSurface::deactivate");
    }
    if (m_Material.m_bHasMask) {
        glproc::ActiveTexture(GL_TEXTURE3);
        glDisable(GL_TEXTURE_2D);
        glproc::ActiveTexture(GL_TEXTURE0);
        glproc::UseProgramObject(0);
    }
}

BitmapPtr OGLSurface::lockBmp(int i)
{
    return m_pTextures[i]->lockBmp();
}

void OGLSurface::unlockBmps()
{
    m_pTextures[0]->unlockBmp();
    if (m_pf == YCbCr420p || m_pf == YCbCrJ420p) {
        m_pTextures[1]->unlockBmp();
        m_pTextures[2]->unlockBmp();
    }
}

BitmapPtr OGLSurface::lockMaskBmp()
{
    assert(m_Material.m_bHasMask);
    return m_pMaskTexture->lockBmp();
}

void OGLSurface::unlockMaskBmp()
{
    m_pMaskTexture->unlockBmp();
}

const MaterialInfo& OGLSurface::getMaterial() const
{
    return m_Material;
}

void OGLSurface::setMaterial(const MaterialInfo& material)
{
    bool bOldHasMask = m_Material.m_bHasMask;
    m_Material = material;
    if (m_pTextures[0]) {
        m_pTextures[0]->setMaterial(material);
        if (m_pf == YCbCr420p || m_pf == YCbCrJ420p) {
            m_pTextures[1]->setMaterial(material);
            m_pTextures[2]->setMaterial(material);
        }
    }
    if (bOldHasMask && !m_Material.m_bHasMask) {
        m_pMaskTexture = OGLTexturePtr();
    }
    if (!bOldHasMask && m_Material.m_bHasMask && m_pMaskTexture) {
        m_pMaskTexture = OGLTexturePtr(new OGLTexture(m_MaskSize, I8, m_Material, 
                m_pEngine, m_MemoryMode));
    }
}

void OGLSurface::downloadTexture()
{
    if (m_pTextures[0]) {
        m_pTextures[0]->download();
        if (m_pf == YCbCr420p || m_pf == YCbCrJ420p) {
            m_pTextures[1]->download();
            m_pTextures[2]->download();
        }
    }
    if (m_Material.m_bHasMask) {
        m_pMaskTexture->download();
    }
}

PixelFormat OGLSurface::getPixelFormat()
{
    return m_pf;
}
        
IntPoint OGLSurface::getSize()
{
    return m_Size;
}

IntPoint OGLSurface::getTextureSize()
{
    return m_pTextures[0]->getTextureSize();
}

SDLDisplayEngine * OGLSurface::getEngine()
{
    return m_pEngine;
}

bool OGLSurface::useShader() const
{
    return (m_Material.m_bHasMask || m_pf == YCbCr420p || m_pf == YCbCrJ420p);
}

}
