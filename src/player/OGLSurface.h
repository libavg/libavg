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

#ifndef _OGLSurface_H_
#define _OGLSurface_H_

#include "../api.h"

#include "../base/GLMHelper.h"
#include "../graphics/PixelFormat.h"
#include "../graphics/WrapMode.h"

#include <boost/shared_ptr.hpp>

namespace avg {

class MCTexture;
typedef boost::shared_ptr<MCTexture> MCTexturePtr;
class GLContext;

class AVG_API OGLSurface {
public:
    OGLSurface(const WrapMode& wrapMode);
    virtual ~OGLSurface();

    virtual void create(PixelFormat pf, MCTexturePtr pTex0, 
            MCTexturePtr pTex1 = MCTexturePtr(), MCTexturePtr pTex2 = MCTexturePtr(), 
            MCTexturePtr pTex3 = MCTexturePtr(), bool bPremultipliedAlpha = false);
    void setMask(MCTexturePtr pTex);
    virtual void destroy();
    void activate(GLContext* pContext, const IntPoint& logicalSize = IntPoint(1,1)) const;

    void setMaskCoords(glm::vec2 maskPos, glm::vec2 maskSize);

    PixelFormat getPixelFormat();
    IntPoint getSize();
    IntPoint getTextureSize();
    bool isCreated() const;
    bool isPremultipliedAlpha() const;

    void setColorParams(const glm::vec3& gamma, const glm::vec3& brightness,
            const glm::vec3& contrast);
    void setAlphaGamma(float gamma);

    bool isDirty() const;
    void setDirty();
    void resetDirty();

private:
    glm::mat4 calcColorspaceMatrix() const;

    MCTexturePtr m_pMCTextures[4];
    IntPoint m_Size;
    PixelFormat m_pf;
    MCTexturePtr m_pMaskMCTexture;
    glm::vec2 m_MaskPos;
    glm::vec2 m_MaskSize;
    bool m_bPremultipliedAlpha;
    WrapMode m_WrapMode;

    glm::vec4 m_Gamma;
    bool m_bColorIsModified;
    glm::vec3 m_Brightness;
    glm::vec3 m_Contrast;

    bool m_bIsDirty;

};

}

#endif

