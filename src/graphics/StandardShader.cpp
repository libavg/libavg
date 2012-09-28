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

#include "StandardShader.h"

#include "GLContext.h"
#include "ShaderRegistry.h"

#include "../base/Logger.h"
#include "../base/Exception.h"

#include <iostream>

using namespace std;

#define STANDARD_SHADER "standard"
#define MINIMAL_SHADER "minimal"

namespace avg {

StandardShaderPtr StandardShader::get() 
{
    return GLContext::getMain()->getStandardShader();
}

StandardShader::StandardShader()
{
    avg::createShader(STANDARD_SHADER);
    m_pShader = avg::getShader(STANDARD_SHADER);
    m_pColorModelParam = m_pShader->getParam<int>("colorModel");
    m_pAlphaParam = m_pShader->getParam<float>("alpha");
    m_pColorCoeff0Param = m_pShader->getParam<glm::vec4>("colorCoeff0");
    m_pColorCoeff1Param = m_pShader->getParam<glm::vec4>("colorCoeff1");
    m_pColorCoeff2Param = m_pShader->getParam<glm::vec4>("colorCoeff2");
    m_pColorCoeff3Param = m_pShader->getParam<glm::vec4>("colorCoeff3");
    m_pGammaParam = m_pShader->getParam<glm::vec4>("gamma");
 
    m_pUseColorCoeffParam = m_pShader->getParam<int>("bUseColorCoeff");
    m_pPremultipliedAlphaParam = m_pShader->getParam<int>("bPremultipliedAlpha");
    m_pUseMaskParam = m_pShader->getParam<int>("bUseMask");
    m_pMaskPosParam = m_pShader->getParam<glm::vec2>("maskPos");
    m_pMaskSizeParam = m_pShader->getParam<glm::vec2>("maskSize");

    m_pShader->activate();
    m_pShader->getParam<int>("texture")->set(0);
    if (GLContext::getMain()->useGPUYUVConversion()) {
        m_pShader->getParam<int>("cbTexture")->set(1);
        m_pShader->getParam<int>("crTexture")->set(2);
        m_pShader->getParam<int>("aTexture")->set(3);
    }
    m_pShader->getParam<int>("maskTexture")->set(4);

    if (GLContext::getMain()->useMinimalShader()) {
        avg::createShader(MINIMAL_SHADER);
        m_pMinimalShader = avg::getShader(MINIMAL_SHADER);
        m_pMinimalShader->activate();
        m_pMinimalShader->getParam<int>("texture")->set(0);
        m_pMinimalAlphaParam = m_pMinimalShader->getParam<float>("alpha");
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
        m_pMinimalAlphaParam->set(m_Alpha);
    } else {
        m_pShader->activate();
        m_pShader->setTransform(m_Transform);
        m_pColorModelParam->set(m_ColorModel);
        m_pAlphaParam->set(m_Alpha);

        m_pUseColorCoeffParam->set(m_bUseColorCoeff);
        const glm::mat4& mat = m_ColorMatrix;
        m_pColorCoeff0Param->set(glm::vec4(mat[0][0], mat[0][1], mat[0][2], 0));
        m_pColorCoeff1Param->set(glm::vec4(mat[1][0], mat[1][1], mat[1][2], 0));
        m_pColorCoeff2Param->set(glm::vec4(mat[2][0], mat[2][1], mat[2][2], 0));
        m_pColorCoeff3Param->set(glm::vec4(mat[3][0], mat[3][1], mat[3][2], 1));
        m_pGammaParam->set(m_Gamma);

        m_pPremultipliedAlphaParam->set(m_bPremultipliedAlpha);

        m_pUseMaskParam->set(m_bUseMask);
        if (m_bUseMask) {
            m_pMaskPosParam->set(m_MaskPos);
            m_pMaskSizeParam->set(m_MaskSize);
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
    m_pWhiteTex->activate(GL_TEXTURE0);
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
    m_pWhiteTex = GLTexturePtr(new GLTexture(IntPoint(1,1), I8));
    m_pWhiteTex->moveBmpToTexture(pBmp);
}

bool StandardShader::useMinimalShader() const
{
    bool bActivateMinimal = false;
    if (GLContext::getMain()->useMinimalShader()) {
        bool bGammaIsModified = (!almostEqual(m_Gamma, glm::vec4(1.0f,1.0f,1.0f,1.0f)));
        if (m_ColorModel == 0 && !m_bUseColorCoeff && !bGammaIsModified && !m_bUseMask) {
            bActivateMinimal = true;
        }
    }
    return bActivateMinimal;
}

}
