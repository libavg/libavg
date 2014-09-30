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

#include "OGLSurface.h"

#include "../base/MathHelper.h"
#include "../base/Logger.h"
#include "../base/Exception.h"
#include "../base/ScopeTimer.h"
#include "../base/ObjectCounter.h"

#include "../graphics/GLContext.h"
#include "../graphics/MCTexture.h"

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

OGLSurface::OGLSurface()
    : m_Size(-1,-1),
      m_Gamma(1,1,1),
      m_Brightness(1,1,1),
      m_Contrast(1,1,1),
      m_AlphaGamma(1),
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
    m_pTextures[0] = pTex0;
    m_pTextures[1] = pTex1;
    m_pTextures[2] = pTex2;
    m_pTextures[3] = pTex3;
    m_bIsDirty = true;
    m_bPremultipliedAlpha = bPremultipliedAlpha;

    // Make sure pixel format and number of textures line up.
    if (pixelFormatIsPlanar(pf)) {
        AVG_ASSERT(m_pTextures[2]);
        if (pixelFormatHasAlpha(m_pf)) {
            AVG_ASSERT(m_pTextures[3]);
        } else {
            AVG_ASSERT(!m_pTextures[3]);
        }
    } else {
        AVG_ASSERT(!m_pTextures[1]);
    }
}

void OGLSurface::setMask(MCTexturePtr pTex)
{
    m_pMaskTexture = pTex;
    m_bIsDirty = true;
}

void OGLSurface::destroy()
{
    m_pTextures[0] = MCTexturePtr();
    m_pTextures[1] = MCTexturePtr();
    m_pTextures[2] = MCTexturePtr();
    m_pTextures[3] = MCTexturePtr();
}

void OGLSurface::activate(const IntPoint& logicalSize) const
{
    StandardShaderPtr pShader = StandardShader::get();

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

    m_pTextures[0]->activate(GL_TEXTURE0);

    if (pixelFormatIsPlanar(m_pf)) {
        m_pTextures[1]->activate(GL_TEXTURE1);
        m_pTextures[2]->activate(GL_TEXTURE2);
        if (m_pf == YCbCrA420p) {
            m_pTextures[3]->activate(GL_TEXTURE3);
        }
    }
    if (pixelFormatIsPlanar(m_pf) || colorIsModified()) {
        glm::mat4 mat = calcColorspaceMatrix();
        pShader->setColorspaceMatrix(mat);
    } else {
        pShader->disableColorspaceMatrix();
    }
    pShader->setGamma(glm::vec4(1/m_Gamma.x, 1/m_Gamma.y, 1/m_Gamma.z, 
                1./m_AlphaGamma));

    pShader->setPremultipliedAlpha(m_bPremultipliedAlpha);
    if (m_pMaskTexture) {
        m_pMaskTexture->activate(GL_TEXTURE4);
        // Special case for pot textures: 
        //   The tex coords in the vertex array are scaled to fit the image texture. We 
        //   need to undo this and fit to the mask texture. In the npot case, everything
        //   evaluates to (1,1);
        glm::vec2 texSize = glm::vec2(m_pTextures[0]->getGLSize());
        glm::vec2 imgSize = glm::vec2(m_pTextures[0]->getSize());
        glm::vec2 maskTexSize = glm::vec2(m_pMaskTexture->getGLSize());
        glm::vec2 maskImgSize = glm::vec2(m_pMaskTexture->getSize());
        glm::vec2 maskScale = glm::vec2(maskTexSize.x/maskImgSize.x, 
                maskTexSize.y/maskImgSize.y);
        glm::vec2 imgScale = glm::vec2(texSize.x/imgSize.x, texSize.y/imgSize.y);
        glm::vec2 maskPos = m_MaskPos/maskScale;
        // Special case for words nodes.
        if (logicalSize != IntPoint(0,0)) {
            maskScale *= glm::vec2((float)logicalSize.x/m_Size.x, 
                    (float)logicalSize.y/m_Size.y);
        }
        pShader->setMask(true, maskPos, m_MaskSize*maskScale/imgScale);
    } else {
        pShader->setMask(false);
    }
    pShader->activate();
    GLContext::checkError("OGLSurface::activate");
}

MCTexturePtr OGLSurface::getTex(int i) const
{
    return m_pTextures[i];
}

void OGLSurface::setMaskCoords(glm::vec2 maskPos, glm::vec2 maskSize)
{
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
    return m_pTextures[0]->getGLSize();
}

bool OGLSurface::isCreated() const
{
    return m_pTextures[0];
}

bool OGLSurface::isPremultipliedAlpha() const
{
    return m_bPremultipliedAlpha;
}

void OGLSurface::setColorParams(const glm::vec3& gamma, const glm::vec3& brightness,
            const glm::vec3& contrast)
{
    m_Gamma = gamma;
    m_Brightness = brightness;
    m_Contrast = contrast;
    m_bIsDirty = true;
}

void OGLSurface::setAlphaGamma(float gamma)
{
    m_AlphaGamma = gamma;
    m_bIsDirty = true;
}

bool OGLSurface::isDirty() const
{
    bool bIsDirty = m_bIsDirty;
    for (unsigned i=0; i<getNumPixelFormatPlanes(m_pf); ++i) {
        if (m_pTextures[i]->isDirty()) {
            bIsDirty = true;
        }
    }
    return bIsDirty;
}

void OGLSurface::resetDirty()
{
    m_bIsDirty = false;
    for (unsigned i=0; i<getNumPixelFormatPlanes(m_pf); ++i) {
        m_pTextures[i]->resetDirty();
    }
}

glm::mat4 OGLSurface::calcColorspaceMatrix() const
{
    glm::mat4 mat;
    if (colorIsModified()) {
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

bool OGLSurface::colorIsModified() const
{
    return (fabs(m_Brightness.x-1.0) > 0.00001 || fabs(m_Brightness.y-1.0) > 0.00001 ||
           fabs(m_Brightness.z-1.0) > 0.00001 || fabs(m_Contrast.x-1.0) > 0.00001 ||
           fabs(m_Contrast.y-1.0) > 0.00001 || fabs(m_Contrast.z-1.0) > 0.00001);
}

}
