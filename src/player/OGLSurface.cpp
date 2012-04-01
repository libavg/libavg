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

#include "OGLSurface.h"

#include "../base/MathHelper.h"
#include "../base/Logger.h"
#include "../base/Exception.h"
#include "../base/ScopeTimer.h"
#include "../base/ObjectCounter.h"

#include "../graphics/ShaderRegistry.h"
#include "../graphics/GLContext.h"
#include "../graphics/GLTexture.h"

#include <iostream>
#include <sstream>

#include "../glm/gtc/matrix_transform.hpp"

using namespace std;

#define COLORSPACE_SHADER "color"

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

void OGLSurface::attach()
{
    if (!GLContext::getCurrent()->isUsingShaders()) {
        if (m_pMaskTexture) {
            throw Exception(AVG_ERR_VIDEO_GENERAL,
                    "Can't set mask bitmap since shader support is disabled.");
        }
        if (gammaIsModified() || colorIsModified()) {
            throw Exception(AVG_ERR_VIDEO_GENERAL,
                    "Can't use color correction (gamma, brightness, contrast) since shader support is disabled.");
        }
    }
}

void OGLSurface::create(PixelFormat pf, GLTexturePtr pTex0, GLTexturePtr pTex1, 
        GLTexturePtr pTex2, GLTexturePtr pTex3)
{
    m_pf = pf;
    m_Size = pTex0->getSize();
    m_pTextures[0] = pTex0;
    m_pTextures[1] = pTex1;
    m_pTextures[2] = pTex2;
    m_pTextures[3] = pTex3;
    m_bIsDirty = true;

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

    m_pShader = getShader(COLORSPACE_SHADER);
    m_pColorModelParam = IntGLShaderParamPtr(new IntGLShaderParam(m_pShader, 
            "colorModel"));
    m_pTextureParam = IntGLShaderParamPtr(new IntGLShaderParam(m_pShader, 
            "texture"));
    m_pCbTextureParam = IntGLShaderParamPtr(new IntGLShaderParam(m_pShader, 
            "cbTexture"));
    m_pCrTextureParam = IntGLShaderParamPtr(new IntGLShaderParam(m_pShader, 
            "crTexture"));
    m_pATextureParam = IntGLShaderParamPtr(new IntGLShaderParam(m_pShader, 
            "aTexture"));
    
    m_pColorCoeff0Param = Vec4fGLShaderParamPtr(new Vec4fGLShaderParam(m_pShader, 
            "colorCoeff0"));
    m_pColorCoeff1Param = Vec4fGLShaderParamPtr(new Vec4fGLShaderParam(m_pShader, 
            "colorCoeff1"));
    m_pColorCoeff2Param = Vec4fGLShaderParamPtr(new Vec4fGLShaderParam(m_pShader, 
            "colorCoeff2"));
    m_pColorCoeff3Param = Vec4fGLShaderParamPtr(new Vec4fGLShaderParam(m_pShader, 
            "colorCoeff3"));
    m_pGammaParam = Vec4fGLShaderParamPtr(new Vec4fGLShaderParam(m_pShader, 
            "gamma"));
 
    m_pUseColorCoeffParam = IntGLShaderParamPtr(new IntGLShaderParam(m_pShader, 
            "bUseColorCoeff"));
    m_pPremultipliedAlphaParam = IntGLShaderParamPtr(new IntGLShaderParam(m_pShader, 
            "bPremultipliedAlpha"));
    m_pUseMaskParam = IntGLShaderParamPtr(new IntGLShaderParam(m_pShader, 
            "bUseMask"));
    m_pMaskTextureParam = IntGLShaderParamPtr(new IntGLShaderParam(m_pShader, 
            "maskTexture"));
    m_pMaskPosParam = Vec2fGLShaderParamPtr(new Vec2fGLShaderParam(m_pShader, 
            "maskPos"));
    m_pMaskSizeParam = Vec2fGLShaderParamPtr(new Vec2fGLShaderParam(m_pShader, 
            "maskSize"));
}

void OGLSurface::setMask(GLTexturePtr pTex)
{
    m_pMaskTexture = pTex;
    m_bIsDirty = true;
}

void OGLSurface::destroy()
{
    m_pTextures[0] = GLTexturePtr();
    m_pTextures[1] = GLTexturePtr();
    m_pTextures[2] = GLTexturePtr();
    m_pTextures[3] = GLTexturePtr();
}

void OGLSurface::activate(const IntPoint& logicalSize, bool bPremultipliedAlpha) const
{
    if (useShader()) {
        m_pShader->activate();
        OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "OGLSurface::activate()");
        switch (m_pf) {
            case YCbCr420p:
            case YCbCrJ420p:
                m_pColorModelParam->set(1);
                break;
            case YCbCrA420p:
                m_pColorModelParam->set(3);
                break;
            case A8:
                m_pColorModelParam->set(2);
                break;
            default:
                m_pColorModelParam->set(0);
        }

        m_pTextures[0]->activate(GL_TEXTURE0);
        m_pTextureParam->set(0);
        
        if (pixelFormatIsPlanar(m_pf)) {
            m_pTextures[1]->activate(GL_TEXTURE1);
            m_pCbTextureParam->set(1);
            m_pTextures[2]->activate(GL_TEXTURE2);
            m_pCrTextureParam->set(2);
            if (m_pf == YCbCrA420p) {
                m_pTextures[3]->activate(GL_TEXTURE3);
                m_pATextureParam->set(3);
            }
        }
        if (pixelFormatIsPlanar(m_pf) || colorIsModified()) {
            glm::mat4 mat = calcColorspaceMatrix();
            m_pColorCoeff0Param->set(glm::vec4(mat[0][0], mat[0][1], mat[0][2], 0));
            m_pColorCoeff1Param->set(glm::vec4(mat[1][0], mat[1][1], mat[1][2], 0));
            m_pColorCoeff2Param->set(glm::vec4(mat[2][0], mat[2][1], mat[2][2], 0));
            m_pColorCoeff3Param->set(glm::vec4(mat[3][0], mat[3][1], mat[3][2], 1));
        }

        m_pGammaParam->set(glm::vec4(1/m_Gamma.x, 1/m_Gamma.y, 1/m_Gamma.z, 
                1./m_AlphaGamma));
        m_pUseColorCoeffParam->set(colorIsModified());

        m_pPremultipliedAlphaParam->set(bPremultipliedAlpha);
        m_pUseMaskParam->set(bool(m_pMaskTexture));
        if (m_pMaskTexture) {
            m_pMaskTexture->activate(GL_TEXTURE4);
            m_pMaskTextureParam->set(4);
            m_pMaskPosParam->set(m_MaskPos);
            // maskScale is (1,1) for everything excepting words nodes.
            glm::vec2 maskScale(1,1);
            if (logicalSize != IntPoint(0,0)) {
                maskScale = glm::vec2((float)logicalSize.x/m_Size.x, 
                        (float)logicalSize.y/m_Size.y);
            }
            m_pMaskSizeParam->set(m_MaskSize*maskScale);
        }

        OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "OGLSurface::activate: params");
    } else {
        m_pTextures[0]->activate(GL_TEXTURE0);
        if (GLContext::getCurrent()->isUsingShaders()) {
            glproc::UseProgramObject(0);
        }
        for (int i=1; i<5; ++i) {
            glproc::ActiveTexture(GL_TEXTURE0 + i);
            glDisable(GL_TEXTURE_2D);
        }
        glproc::ActiveTexture(GL_TEXTURE0);
        OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "OGLSurface::activate: fixed function");
    }
}

GLTexturePtr OGLSurface::getTex(int i) const
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

void OGLSurface::setColorParams(const glm::vec3& gamma, const glm::vec3& brightness,
            const glm::vec3& contrast)
{
    m_Gamma = gamma;
    m_Brightness = brightness;
    m_Contrast = contrast;
    if (!GLContext::getCurrent()->isUsingShaders() &&
            (gammaIsModified() || colorIsModified())) 
    {
        throw Exception(AVG_ERR_VIDEO_GENERAL,
                "Can't use color correction (gamma, brightness, contrast) since shader support is disabled.");
    }
    m_bIsDirty = true;
}

void OGLSurface::setAlphaGamma(float gamma)
{
    m_AlphaGamma = gamma;
    if (!GLContext::getCurrent()->isUsingShaders() &&
            (gammaIsModified() || colorIsModified())) 
    {
        throw Exception(AVG_ERR_VIDEO_GENERAL,
                "Can't use color correction (gamma, brightness, contrast) since shader support is disabled.");
    }
    m_bIsDirty = true;
}

void OGLSurface::createShader()
{
    avg::createShader(COLORSPACE_SHADER);
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

bool OGLSurface::useShader() const
{
    return GLContext::getCurrent()->isUsingShaders() && 
            (m_pMaskTexture || pixelFormatIsPlanar(m_pf) || gammaIsModified() || 
                    colorIsModified());
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

bool OGLSurface::gammaIsModified() const
{
    return (!almostEqual(m_Gamma.x, 1.0f) || !almostEqual(m_Gamma.y, 1.0f) ||
            !almostEqual(m_Gamma.z, 1.0f) || !almostEqual(m_AlphaGamma, 1.0f));
}

bool OGLSurface::colorIsModified() const
{
    return (fabs(m_Brightness.x-1.0) > 0.00001 || fabs(m_Brightness.y-1.0) > 0.00001 ||
           fabs(m_Brightness.z-1.0) > 0.00001 || fabs(m_Contrast.x-1.0) > 0.00001 ||
           fabs(m_Contrast.y-1.0) > 0.00001 || fabs(m_Contrast.z-1.0) > 0.00001);
}

}
