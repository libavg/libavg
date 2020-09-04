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

#include "StandardShader.h"

#include "GLContext.h"
#include "OGLShader.h"
#include "GLTexture.h"
#include "ShaderRegistry.h"

#include "../base/Logger.h"
#include "../base/Exception.h"

#include <iostream>

using namespace std;

#define STANDARD_SHADER "standard"
#define MINIMAL_SHADER "minimal"

namespace avg {

StandardShader::StandardShader(GLContext* pContext)
    : m_pContext(pContext)
{
    m_pContext->getShaderRegistry()->createShader(STANDARD_SHADER);
    m_pShader = avg::getShader(STANDARD_SHADER);
    m_ColorModelParam = *m_pShader->getParam<int>("u_ColorModel");
    m_AlphaParam = *m_pShader->getParam<float>("u_Alpha");
    m_ColorCoeff0Param = *m_pShader->getParam<glm::vec4>("u_ColorCoeff0");
    m_ColorCoeff1Param = *m_pShader->getParam<glm::vec4>("u_ColorCoeff1");
    m_ColorCoeff2Param = *m_pShader->getParam<glm::vec4>("u_ColorCoeff2");
    m_ColorCoeff3Param = *m_pShader->getParam<glm::vec4>("u_ColorCoeff3");
    m_GammaParam = *m_pShader->getParam<glm::vec4>("u_Gamma");
 
    m_UseColorCoeffParam = *m_pShader->getParam<int>("u_bUseColorCoeff");
    m_PremultipliedAlphaParam = *m_pShader->getParam<int>("u_bPremultipliedAlpha");
    m_UseMaskParam = *m_pShader->getParam<int>("u_bUseMask");
    m_MaskPosParam = *m_pShader->getParam<glm::vec2>("u_MaskPos");
    m_MaskSizeParam = *m_pShader->getParam<glm::vec2>("u_MaskSize");

    m_pShader->activate();
    m_pShader->getParam<int>("u_Texture")->set(0);
    if (m_pContext->useGPUYUVConversion()) {
        m_pShader->getParam<int>("u_CBTexture")->set(1);
        m_pShader->getParam<int>("u_CRTexture")->set(2);
        m_pShader->getParam<int>("u_ATexture")->set(3);
    }
    m_pShader->getParam<int>("u_MaskTexture")->set(4);

    if (m_pContext->getShaderUsage() != GLConfig::FULL) {
        m_pContext->getShaderRegistry()->createShader(MINIMAL_SHADER);
        m_pMinimalShader = avg::getShader(MINIMAL_SHADER);
        m_pMinimalShader->activate();
        m_pMinimalShader->getParam<int>("u_Texture")->set(0);
        m_MinimalAlphaParam = *m_pMinimalShader->getParam<float>("u_Alpha");
    }
    
    generateWhiteTexture();
}

StandardShader::~StandardShader()
{
}

void StandardShader::activate()
{
    if (useMinimalShader()) {
        m_pMinimalShader->activate();
        m_pMinimalShader->setTransform(m_Transform);
        m_MinimalAlphaParam.set(m_Alpha);
    } else {
        m_pShader->activate();
        m_pShader->setTransform(m_Transform);
        m_ColorModelParam.set(m_ColorModel);
        m_AlphaParam.set(m_Alpha);

        m_UseColorCoeffParam.set(m_bUseColorCoeff);
        const glm::mat4& mat = m_ColorMatrix;
        m_ColorCoeff0Param.set(glm::vec4(mat[0][0], mat[0][1], mat[0][2], 0));
        m_ColorCoeff1Param.set(glm::vec4(mat[1][0], mat[1][1], mat[1][2], 0));
        m_ColorCoeff2Param.set(glm::vec4(mat[2][0], mat[2][1], mat[2][2], 0));
        m_ColorCoeff3Param.set(glm::vec4(mat[3][0], mat[3][1], mat[3][2], 1));
        m_GammaParam.set(m_Gamma);

        m_PremultipliedAlphaParam.set(m_bPremultipliedAlpha);

        m_UseMaskParam.set(m_bUseMask);
        if (m_bUseMask) {
            m_MaskPosParam.set(m_MaskPos);
            m_MaskSizeParam.set(m_MaskSize);
        }
    }
}

void StandardShader::setTransform(const glm::mat4& transform)
{
    m_Transform = transform;
}

void StandardShader::setColorModel(int model)
{
    m_ColorModel = model;
}

void StandardShader::setAlpha(float alpha)
{
    m_Alpha = alpha;
}
    
void StandardShader::setUntextured()
{
    // Activate an internal 1x1 A8 texture.
    m_ColorModel = 2;
    m_pWhiteTex->activate(WrapMode(), GL_TEXTURE0);
    disableColorspaceMatrix();
    setGamma(glm::vec4(1.f,1.f,1.f,1.f));
    setPremultipliedAlpha(false);
    setMask(false);
}

void StandardShader::setColorspaceMatrix(const glm::mat4& mat)
{
    m_bUseColorCoeff = true;
    m_ColorMatrix = mat;
}

void StandardShader::disableColorspaceMatrix()
{
    m_bUseColorCoeff = false;
}

void StandardShader::setGamma(const glm::vec4& gamma)
{
    m_Gamma = gamma;
}

void StandardShader::setPremultipliedAlpha(bool bPremultipliedAlpha)
{
    m_bPremultipliedAlpha = bPremultipliedAlpha;
}

void StandardShader::setMask(bool bUseMask, const glm::vec2& maskPos,
        const glm::vec2& maskSize)
{
    m_bUseMask = bUseMask;
    m_MaskPos = maskPos;
    m_MaskSize = maskSize;
}

const OGLShaderPtr& StandardShader::getShader() const
{
    if (useMinimalShader()) {
        return m_pMinimalShader;
    } else {
        return m_pShader;
    }
}

void StandardShader::dump() const
{
    cerr << "---------Standard shader--------" << endl;
    cerr << "  m_Transform: " << m_Transform << endl;
    cerr << "  m_ColorModel: " << m_ColorModel << endl;
    cerr << "  m_Alpha: " << m_Alpha << endl;
    cerr << "  m_bUseColorCoeff: " << m_bUseColorCoeff << endl;
    cerr << "  m_ColorMatrix: " << m_ColorMatrix << endl;
    cerr << "  m_Gamma: " << m_Gamma << endl;
    cerr << "  m_bPremultipliedAlpha: " << m_bPremultipliedAlpha << endl;
    cerr << "  m_bUseMask: " << m_bUseMask << endl;
    cerr << "  m_MaskPos: " << m_MaskPos << endl;
    cerr << "  m_MaskSize: " << m_MaskSize << endl;
    cerr << endl;
}

void StandardShader::generateWhiteTexture()
{
    BitmapPtr pBmp(new Bitmap(glm::vec2(1,1), I8));
    *(pBmp->getPixels()) = 255;
    m_pWhiteTex = GLTexturePtr(new GLTexture(m_pContext, IntPoint(1,1), I8));
    m_pWhiteTex->moveBmpToTexture(pBmp);
}

bool StandardShader::useMinimalShader() const
{
    bool bActivateMinimal = false;
    if (m_pContext->getShaderUsage() != GLConfig::FULL) {
        bool bGammaIsModified = (!almostEqual(m_Gamma, glm::vec4(1.0f,1.0f,1.0f,1.0f)));
        if (m_ColorModel == 0 && !m_bUseColorCoeff && !bGammaIsModified && !m_bUseMask) {
            bActivateMinimal = true;
        }
    }
    return bActivateMinimal;
}

}
