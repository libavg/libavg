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

#ifndef _StandardShader_H_
#define _StandardShader_H_

#include "../api.h"
#include "GLShaderParam.h"

#include "../base/GLMHelper.h"

#include <boost/shared_ptr.hpp>

#include <string>

namespace avg {

class StandardShader;
typedef boost::shared_ptr<StandardShader> StandardShaderPtr;
class OGLShader;
typedef boost::shared_ptr<OGLShader> OGLShaderPtr;
class GLTexture;
typedef boost::shared_ptr<GLTexture> GLTexturePtr;

class AVG_API StandardShader {
public:
    static StandardShader* get();
    StandardShader();
    virtual ~StandardShader();

    void activate();

    void setTransform(const glm::mat4& transform);
    void setColorModel(int model);
    void setAlpha(float alpha);
    void setUntextured();
    void setColorspaceMatrix(const glm::mat4& mat);
    void disableColorspaceMatrix();
    void setGamma(const glm::vec4& gamma);
    void setPremultipliedAlpha(bool bPremultipliedAlpha);
    void setMask(bool bUseMask, const glm::vec2& maskPos = glm::vec2(0,0),
        const glm::vec2& maskSize = glm::vec2(0,0));

    const OGLShaderPtr& getShader() const;

    void dump() const;

private:
    void generateWhiteTexture();
    bool useMinimalShader() const;

    GLTexturePtr m_pWhiteTex;

    glm::mat4 m_Transform;
    int m_ColorModel;
    float m_Alpha;
    bool m_bUseColorCoeff;
    glm::mat4 m_ColorMatrix;
    glm::vec4 m_Gamma;
    bool m_bPremultipliedAlpha;
    bool m_bUseMask;
    glm::vec2 m_MaskPos;
    glm::vec2 m_MaskSize;

    OGLShaderPtr m_pShader;
    OGLShaderPtr m_pMinimalShader;

    IntGLShaderParamPtr m_pColorModelParam;
    FloatGLShaderParamPtr m_pAlphaParam;

    Vec4fGLShaderParamPtr m_pColorCoeff0Param;
    Vec4fGLShaderParamPtr m_pColorCoeff1Param;
    Vec4fGLShaderParamPtr m_pColorCoeff2Param;
    Vec4fGLShaderParamPtr m_pColorCoeff3Param;
    Vec4fGLShaderParamPtr m_pGammaParam;

    IntGLShaderParamPtr m_pUseColorCoeffParam;
    IntGLShaderParamPtr m_pPremultipliedAlphaParam;
    IntGLShaderParamPtr m_pUseMaskParam;
    Vec2fGLShaderParamPtr m_pMaskPosParam;
    Vec2fGLShaderParamPtr m_pMaskSizeParam;

    FloatGLShaderParamPtr m_pMinimalAlphaParam;
};

}

#endif

