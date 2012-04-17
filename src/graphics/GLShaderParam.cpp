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

#include "GLShaderParam.h"
#include "OGLShader.h"
#include <iostream>

using namespace std;

namespace avg {

GLShaderParam::GLShaderParam(OGLShader* pShader, const std::string& sName)
    : m_sName(sName)
{
    m_Location = glproc::GetUniformLocation(pShader->getProgram(), sName.c_str());
    string sErr = std::string("Shader param '") + sName + "' not found in shader '" + 
            pShader->getName() + "'.";
    AVG_ASSERT_MSG(m_Location != -1, sErr.c_str());
    GLContext::getCurrent()->checkError(sErr.c_str());
};

int GLShaderParam::getLocation() const
{
    return m_Location;
}

const string& GLShaderParam::getName() const
{
    return m_sName;
}

template<>
void GLShaderParamTemplate<int>::uniformSet(unsigned location, int val)
{
    glproc::Uniform1i(location, val);
}

template<>
void GLShaderParamTemplate<float>::uniformSet(unsigned location, float val)
{
    glproc::Uniform1f(location, val);
}

template<>
void GLShaderParamTemplate<glm::vec2>::uniformSet(unsigned location, glm::vec2 val)
{
    glproc::Uniform2f(location, val.x, val.y);
}

template<>
void GLShaderParamTemplate<Pixel32>::uniformSet(unsigned location, Pixel32 val)
{
    glproc::Uniform4f(location, val.getR()/255.f, val.getG()/255.f, val.getB()/255.f,
            val.getA()/255.f);
}

template<>
void GLShaderParamTemplate<glm::vec4>::uniformSet(unsigned location, glm::vec4 val)
{
    glproc::Uniform4f(location, val[0], val[1], val[2], val[3]);
}

}
