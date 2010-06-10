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

#include "NullFXNode.h"
#include "SDLDisplayEngine.h"

#include "../base/ObjectCounter.h"

#include <string>

using namespace std;

namespace avg {

OGLShaderPtr NullFXNode::s_pShader;

NullFXNode::NullFXNode() 
    : FXNode()
{
    ObjectCounter::get()->incRef(&typeid(*this));
}

NullFXNode::~NullFXNode()
{
    ObjectCounter::get()->decRef(&typeid(*this));
}

void NullFXNode::connect(SDLDisplayEngine* pEngine)
{
    FXNode::connect(pEngine);
    initShader();
}

void NullFXNode::disconnect()
{
    FXNode::disconnect();
}

void NullFXNode::setSize(const IntPoint& newSize)
{
    FXNode::setSize(newSize);
}

void NullFXNode::apply(GLTexturePtr pSrcTex)
{
    s_pShader->activate();
    s_pShader->setUniformIntParam("texture", 0);

    // Shader overwrites everything, so no glClear necessary before.
    glproc::BlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_ONE, GL_ZERO);
    FBOPtr pFBO = getFBO();
    pSrcTex->activate();
    pFBO->activate();
    pFBO->setupImagingProjection();
    pFBO->drawImagingVertexes();
    pFBO->deactivate();
    glproc::UseProgramObject(0);
    pFBO->copyToDestTexture();
}

void NullFXNode::initShader()
{
    if (!s_pShader) {
        string sProgram =
            "uniform sampler2D texture;\n"

            "void main(void)\n"
            "{\n"
            "  vec4 tex = texture2D(texture, gl_TexCoord[0].st);\n" 
            "  gl_FragColor.rgba = tex.rgba;\n"
            "}\n"
            ;

        s_pShader = OGLShaderPtr(new OGLShader(sProgram));
    }
}

}
