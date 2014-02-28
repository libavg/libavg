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

#ifndef _MCShaderParam_H_
#define _MCShaderParam_H_

#include "../api.h"
#include "GLContext.h"
#include "GLShaderParam.h"
#include "OGLShader.h"

#include "../base/GLMHelper.h"
#include "../base/Exception.h"

#include <boost/shared_ptr.hpp>

#include <string>

namespace avg {

class OGLShader;
typedef boost::shared_ptr<OGLShader> OGLShaderPtr;

class AVG_API MCShaderParam
{
public:
    virtual void initForGLContext() = 0;

protected:
    OGLShaderPtr findShader(GLContext* pContext, const std::string sShaderName, 
            const std::string& sParamName);
};


template<class VAL_TYPE>
class AVG_TEMPLATE_API MCShaderParamTemplate: public MCShaderParam
{
    typedef boost::shared_ptr<GLShaderParamTemplate<VAL_TYPE> > GL_SHADER_PARAM_TYPE_PTR;
    typedef std::map<GLContext*, GL_SHADER_PARAM_TYPE_PTR> PARAM_MAP;
public:
    MCShaderParamTemplate(const std::string& sShaderName, const std::string& sParamName)
        : m_sShaderName(sShaderName),
          m_sParamName(sParamName)
    {};
    
    virtual ~MCShaderParamTemplate()
    {};

    void initForGLContext()
    {
        GLContext* pContext = GLContext::getCurrent();
        AVG_ASSERT(m_pParams.count(pContext) == 0);
        OGLShaderPtr pShader = findShader(pContext, m_sShaderName, m_sParamName);
        m_pParams[pContext] = pShader->getParam<VAL_TYPE>(m_sParamName);
    };

    void set(const VAL_TYPE& val)
    {
        typename PARAM_MAP::iterator it = m_pParams.find(GLContext::getCurrent());
        AVG_ASSERT(it != m_pParams.end());
        GL_SHADER_PARAM_TYPE_PTR pParam = it->second;
        pParam->set(val);
    };

private:
    
    std::string m_sShaderName;
    std::string m_sParamName;
    std::map<GLContext*, GL_SHADER_PARAM_TYPE_PTR> m_pParams;

};

typedef boost::shared_ptr<MCShaderParam> MCShaderParamPtr;

typedef MCShaderParamTemplate<int> IntMCShaderParam;
typedef boost::shared_ptr<IntMCShaderParam> IntMCShaderParamPtr;

typedef MCShaderParamTemplate<float> FloatMCShaderParam; 
typedef boost::shared_ptr<FloatMCShaderParam> FloatMCShaderParamPtr;

typedef MCShaderParamTemplate<glm::vec2> Vec2fMCShaderParam; 
typedef boost::shared_ptr<Vec2fMCShaderParam> Vec2fMCShaderParamPtr;

typedef MCShaderParamTemplate<Pixel32> ColorMCShaderParam; 
typedef boost::shared_ptr<ColorMCShaderParam> ColorMCShaderParamPtr;

typedef MCShaderParamTemplate<glm::vec4> Vec4fMCShaderParam; 
typedef boost::shared_ptr<Vec4fMCShaderParam> Vec4fMCShaderParamPtr;

typedef MCShaderParamTemplate<glm::mat4> Mat4fMCShaderParam; 
typedef boost::shared_ptr<Mat4fMCShaderParam> Mat4fMCShaderParamPtr;

}

#endif

