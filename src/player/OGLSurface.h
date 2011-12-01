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

#ifndef _OGLSurface_H_
#define _OGLSurface_H_

#include "../api.h"

#include "../base/GLMHelper.h"

#include "../graphics/Bitmap.h"
#include "../graphics/OGLHelper.h"

#include <vector>
#include <string>

namespace avg {

class GLTexture;
typedef boost::shared_ptr<GLTexture> GLTexturePtr;


class AVG_API OGLSurface {
public:
    OGLSurface();
    virtual ~OGLSurface();

    void attach();
    virtual void create(PixelFormat pf, GLTexturePtr pTex0, 
            GLTexturePtr pTex1 = GLTexturePtr(), GLTexturePtr pTex2 = GLTexturePtr(), 
            GLTexturePtr pTex3 = GLTexturePtr());
    void setMask(GLTexturePtr pTex);
    virtual void destroy();
    void activate(const IntPoint& logicalSize = IntPoint(1,1),
            bool bPremultipliedAlpha = false) const;
    GLTexturePtr getTex(int i=0) const;

    void setMaskCoords(glm::vec2 maskPos, glm::vec2 maskSize);

    PixelFormat getPixelFormat();
    IntPoint getSize();
    IntPoint getTextureSize();
    bool isCreated() const;

    void setColorParams(const glm::vec3& gamma, const glm::vec3& brightness,
            const glm::vec3& contrast);
    static void createShader();

    bool isDirty() const;
    void resetDirty();

private:
    bool useShader() const;
    glm::mat4 calcColorspaceMatrix() const;
    bool gammaIsModified() const;
    bool colorIsModified() const;

    GLTexturePtr m_pTextures[4];
    IntPoint m_Size;
    PixelFormat m_pf;
    GLTexturePtr m_pMaskTexture;
    glm::vec2 m_MaskPos;
    glm::vec2 m_MaskSize;
    
    glm::vec3 m_Gamma;
    glm::vec3 m_Brightness;
    glm::vec3 m_Contrast;

    bool m_bIsDirty;
};

}

#endif

