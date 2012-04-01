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

#ifndef _GLShaderParamTemplate_H_
#define _GLShaderParamTemplate_H_

#include "../api.h"
#include "OGLHelper.h"
#include "OGLShader.h"
#include "Pixel32.h"

#include "../base/GLMHelper.h"
#include "../base/Exception.h"

#include <boost/shared_ptr.hpp>

#include <string>

namespace avg {

class GLShaderParam
{
public:
    GLShaderParam(OGLShaderPtr pShader, const std::string& sName);
    virtual ~GLShaderParam() {};

protected:
    unsigned getLocation() const;

private:
    OGLShaderPtr m_pShader;
    std::string m_sName;
    unsigned m_Location;
};


template<class VAL_TYPE>
class AVG_TEMPLATE_API GLShaderParamTemplate: public GLShaderParam
{
public:
    GLShaderParamTemplate(OGLShaderPtr pShader, const std::string& sName)
        : GLShaderParam(pShader, sName)
    {};
    
    void set(VAL_TYPE val)
    {
        if (m_Val != val) {
            uniformSet(m_Location, val);
            OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "OGLShaderParam::set");
            m_Val = val;
        }
    };

private:
    virtual void uniformSet(unsigned location, VAL_TYPE val) {};

    VAL_TYPE m_Val;
};


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
void GLShaderParamTemplate<vec4f>::uniformSet(unsigned location, vec4f val)
{
    glproc::Uniform4f(loc, val[0], val[1], val[2], val[3]);
}

typedef boost::shared_ptr<GLShaderParam> GLShaderParamPtr;

}

#endif

