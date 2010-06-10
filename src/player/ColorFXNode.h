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

#ifndef _ColorFXNode_H_
#define _ColorFXNode_H_

#include "../api.h"

#include "FXNode.h"
#include "../graphics/FBO.h"
#include "../graphics/OGLShader.h"

#include <boost/shared_ptr.hpp>

namespace avg {

class SDLDisplayEngine;

class AVG_API ColorFXNode: public FXNode {
public:
    ColorFXNode();
    virtual ~ColorFXNode();

    virtual void connect(SDLDisplayEngine* pEngine);
    virtual void disconnect();
    virtual void setSize(const IntPoint& newSize);
    void setParams(float brightness, float contrast, float rGamma, float gGamma, 
            float bGamma);

    virtual void apply(GLTexturePtr pSrcTex);

private:
    void initShader();

    float m_Brightness;
    float m_Contrast;
    float m_RGamma;
    float m_GGamma;
    float m_BGamma;

    static OGLShaderPtr s_pShader;
};

typedef boost::shared_ptr<ColorFXNode> ColorFXNodePtr;

}

#endif

