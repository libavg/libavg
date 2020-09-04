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

#include "OGLSurface.h"

#include "../base/MathHelper.h"
#include "../base/Logger.h"
#include "../base/Exception.h"
#include "../base/ScopeTimer.h"
#include "../base/ObjectCounter.h"

#include "../graphics/GLContext.h"
#include "../graphics/MCTexture.h"
#include "../graphics/GLTexture.h"
#include "../graphics/StandardShader.h"

#include <iostream>
#include <sstream>

#include "../glm/gtc/matrix_transform.hpp"

using namespace std;

static glm::mat4 yuvCoeff(
        1.0f,   1.0f,    1.0f,  0.0f,
        0.0f, -0.34f,   1.77f,  0.0f,
       1.40f, -0.71f,    0.0f,  0.0f,
        0.0f,   0.0f,    0.0f,  1.0f);

namespace avg {

OGLSurface::OGLSurface(const WrapMode& wrapMode)
    : m_Size(-1,-1),
      m_WrapMode(wrapMode),
      m_Gamma(1,1,1,1),
      m_Brightness(1,1,1),
      m_Contrast(1,1,1),
      m_bIsDirty(true)
{
    ObjectCounter::get()->incRef(&typeid(*this));
}

OGLSurface::~OGLSurface()
{
    ObjectCounter::get()->decRef(&typeid(*this));
}

void OGLSurface::create(PixelFormat pf, MCTexturePtr pTex0, MCTexturePtr pTex1, 
        MCTexturePtr pTex2, MCTexturePtr pTex3, bool bPremultipliedAlpha)
{
    m_pf = pf;
    m_Size = pTex0->getSize();
    m_pMCTextures[0] = pTex0;
    m_pMCTextures[1] = pTex1;
    m_pMCTextures[2] = pTex2;
    m_pMCTextures[3] = pTex3;
    m_bIsDirty = true;
    m_bPremultipliedAlpha = bPremultipliedAlpha;

    // Make sure pixel format and number of textures line up.
    if (pixelFormatIsPlanar(pf)) {
        AVG_ASSERT(m_pMCTextures[2]);
        if (pixelFormatHasAlpha(m_pf)) {
            AVG_ASSERT(m_pMCTextures[3]);
        } else {
            AVG_ASSERT(!m_pMCTextures[3]);
        }
    } else {
        AVG_ASSERT(!m_pMCTextures[1]);
    }
}

void OGLSurface::setMask(MCTexturePtr pTex)
{
    m_pMaskMCTexture = pTex;
    m_bIsDirty = true;
}

void OGLSurface::destroy()
{
    m_pMCTextures[0] = MCTexturePtr();
    m_pMCTextures[1] = MCTexturePtr();
    m_pMCTextures[2] = MCTexturePtr();
    m_pMCTextures[3] = MCTexturePtr();
}

void OGLSurface::activate(GLContext* pContext, const IntPoint& logicalSize) const
{
    StandardShader* pShader = pContext->getStandardShader();

    GLContext::checkError("OGLSurface::activate()");
    switch (m_pf) {
        case YCbCr420p:
        case YCbCrJ420p:
            pShader->setColorModel(1);
            break;
        case YCbCrA420p:
            pShader->setColorModel(3);
            break;
        case A8:
            pShader->setColorModel(2);
            break;
        default:
            pShader->setColorModel(0);
    }

    m_pMCTextures[0]->getTex(pContext)->activate(m_WrapMode, GL_TEXTURE0);

    if (pixelFormatIsPlanar(m_pf)) {
        m_pMCTextures[1]->getTex(pContext)->activate(m_WrapMode, GL_TEXTURE1);
        m_pMCTextures[2]->getTex(pContext)->activate(m_WrapMode, GL_TEXTURE2);
        if (m_pf == YCbCrA420p) {
            m_pMCTextures[3]->getTex(pContext)->activate(m_WrapMode, GL_TEXTURE3);
        }
    }
    if (pixelFormatIsPlanar(m_pf) || m_bColorIsModified) {
        glm::mat4 mat = calcColorspaceMatrix();
        pShader->setColorspaceMatrix(mat);
    } else {
        pShader->disableColorspaceMatrix();
    }
    pShader->setGamma(m_Gamma);

    pShader->setPremultipliedAlpha(m_bPremultipliedAlpha);
    if (m_pMaskMCTexture) {
        m_pMaskMCTexture->getTex(pContext)->activate(m_WrapMode, GL_TEXTURE4);
        // The shader maskpos param takes the position in texture coordinates (0..1) of 
        // the main texture.
        glm::vec2 maskPos = m_MaskPos;
        glm::vec2 maskSize = m_MaskSize;

        // Special case for pot textures: 
        //   The tex coords in the vertex array are scaled to fit the image texture. We 
        //   need to a) undo this and b) adjust for pot mask textures. In the npot case,
        //   everything evaluates to (1,1);
        glm::vec2 texSize = m_pMCTextures[0]->getGLSize();
        glm::vec2 imgSize = m_pMCTextures[0]->getSize();
        glm::vec2 imgScale = glm::vec2(texSize.x/imgSize.x, texSize.y/imgSize.y);
        maskPos = maskPos/imgScale;
        maskSize = maskSize/imgScale;

        glm::vec2 maskTexSize = m_pMaskMCTexture->getGLSize();
        glm::vec2 maskImgSize = m_pMaskMCTexture->getSize();
        glm::vec2 maskScale = glm::vec2(maskTexSize.x/maskImgSize.x, 
                maskTexSize.y/maskImgSize.y);
        maskPos = maskPos*maskScale;
        maskSize = maskSize*maskScale;

        pShader->setMask(true, maskPos, maskSize);
    } else {
        pShader->setMask(false);
    }
    GLContext::checkError("OGLSurface::activate");
}

void OGLSurface::setMaskCoords(glm::vec2 maskPos, glm::vec2 maskSize)
{
    // Mask coords are normalized to 0..1 over the main image size.
    m_MaskPos = maskPos;
    m_MaskSize = maskSize;
    m_bIsDirty = true;
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
    return m_pMCTextures[0]->getGLSize();
}

bool OGLSurface::isCreated() const
{
    return (m_pMCTextures[0] != MCTexturePtr());
}

bool OGLSurface::isPremultipliedAlpha() const
{
    return m_bPremultipliedAlpha;
}

void OGLSurface::setColorParams(const glm::vec3& gamma, const glm::vec3& brightness,
            const glm::vec3& contrast)
{
    m_Gamma = glm::vec4(1.f/gamma.x, 1.f/gamma.y, 1.f/gamma.z, m_Gamma.w);
    m_Brightness = brightness;
    m_Contrast = contrast;
    m_bColorIsModified = (fabs(m_Brightness.x-1.0) > 0.00001 ||
            fabs(m_Brightness.y-1.0) > 0.00001 || fabs(m_Brightness.z-1.0) > 0.00001 ||
            fabs(m_Contrast.x-1.0) > 0.00001 || fabs(m_Contrast.y-1.0) > 0.00001 ||
            fabs(m_Contrast.z-1.0) > 0.00001);
    m_bIsDirty = true;
}

void OGLSurface::setAlphaGamma(float gamma)
{
    m_Gamma.w = 1.f/gamma;
    m_bIsDirty = true;
}

bool OGLSurface::isDirty() const
{
    bool bIsDirty = m_bIsDirty;
    for (unsigned i=0; i<getNumPixelFormatPlanes(m_pf); ++i) {
        if (m_pMCTextures[i]->isDirty()) {
            bIsDirty = true;
        }
    }
    return bIsDirty;
}

void OGLSurface::setDirty()
{
    m_bIsDirty = true;
}

void OGLSurface::resetDirty()
{
    m_bIsDirty = false;
    for (unsigned i=0; i<getNumPixelFormatPlanes(m_pf); ++i) {
        m_pMCTextures[i]->resetDirty();
    }
}

glm::mat4 OGLSurface::calcColorspaceMatrix() const
{
    glm::mat4 mat;
    if (m_bColorIsModified) {
        mat = glm::scale(mat, m_Brightness);
        glm::vec3 contrast = glm::vec3(0.5f, 0.5f, 0.5f) - m_Contrast/2.f;
        mat = glm::translate(mat, contrast);
        mat = glm::scale(mat, m_Contrast);
    }
    if (m_pf == YCbCr420p || m_pf == YCbCrJ420p || m_pf == YCbCrA420p) {
        mat *= yuvCoeff;
        mat = glm::translate(mat, glm::vec3(0.0, -0.5, -0.5));
        if (m_pf == YCbCr420p || m_pf == YCbCrA420p) {
            mat = glm::scale(mat, 
                    glm::vec3(255.0f/(235-16), 255.0f/(235-16), 255.0f/(235-16)));
            mat = glm::translate(mat, glm::vec3(-16.0f/255, -16.0f/255, -16.0f/255));
        }
    }
    return mat;
}

}
