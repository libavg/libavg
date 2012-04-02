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

#ifndef _GLShaderParam_H_
#define _GLShaderParam_H_

#include "../api.h"
#include "OGLHelper.h"
#include "Pixel32.h"

#include "../base/GLMHelper.h"
#include "../base/Exception.h"

#include <boost/shared_ptr.hpp>

#include <string>

namespace avg {

class OGLShader;
typedef boost::shared_ptr<OGLShader> OGLShaderPtr;

class AVG_API GLShaderParam
{
public:
    GLShaderParam(OGLShader* pShader, const std::string& sName);
    virtual ~GLShaderParam() {};
    
    const std::string& getName() const;

protected:
    unsigned getLocation() const;

private:
    std::string m_sName;
    unsigned m_Location;
};


template<class VAL_TYPE>
class AVG_TEMPLATE_API GLShaderParamTemplate: public GLShaderParam
{
public:
    GLShaderParamTemplate(OGLShader* pShader, const std::string& sName)
        : GLShaderParam(pShader, sName),
          m_bValSet(false)
    {};
    
    void set(VAL_TYPE val)
    {
        if (m_Val != val || !m_bValSet) {
            uniformSet(getLocation(), val);
            OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "OGLShaderParam::set");
            m_Val = val;
            m_bValSet = true;
        }
    };

private:
    void uniformSet(unsigned location, VAL_TYPE val) 
    {
        AVG_ASSERT_MSG(false, 
                getName()+"GLShaderParam::uniformSet() called for unsupported type.");
    };

    bool m_bValSet;
    VAL_TYPE m_Val;
};

template<>
void GLShaderParamTemplate<int>::uniformSet(unsigned location, int val);
template<>
void GLShaderParamTemplate<float>::uniformSet(unsigned location, float val);
template<>
void GLShaderParamTemplate<glm::vec2>::uniformSet(unsigned location, glm::vec2 val);
template<>
void GLShaderParamTemplate<Pixel32>::uniformSet(unsigned location, Pixel32 val);
template<>
void GLShaderParamTemplate<glm::vec4>::uniformSet(unsigned location, glm::vec4 val);

typedef boost::shared_ptr<GLShaderParam> GLShaderParamPtr;

typedef GLShaderParamTemplate<int> IntGLShaderParam;
typedef boost::shared_ptr<IntGLShaderParam> IntGLShaderParamPtr;

typedef GLShaderParamTemplate<float> FloatGLShaderParam; 
typedef boost::shared_ptr<FloatGLShaderParam> FloatGLShaderParamPtr;

typedef GLShaderParamTemplate<glm::vec2> Vec2fGLShaderParam; 
typedef boost::shared_ptr<Vec2fGLShaderParam> Vec2fGLShaderParamPtr;

typedef GLShaderParamTemplate<Pixel32> ColorGLShaderParam; 
typedef boost::shared_ptr<ColorGLShaderParam> ColorGLShaderParamPtr;

typedef GLShaderParamTemplate<glm::vec4> Vec4fGLShaderParam; 
typedef boost::shared_ptr<Vec4fGLShaderParam> Vec4fGLShaderParamPtr;

}

#endif

