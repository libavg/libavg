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

#ifndef _OGLShader_H_
#define _OGLShader_H_

#include "../api.h"
#include "OGLHelper.h"
#include "GLShaderParam.h"
#include "Pixel32.h"

#include "../base/GLMHelper.h"

#include <boost/shared_ptr.hpp>

#include <string>
#include <vector>

namespace avg {

class ShaderRegistry;
typedef boost::shared_ptr<ShaderRegistry> ShaderRegistryPtr;

class AVG_API OGLShader {
    public:
        virtual ~OGLShader();

        void activate();
        GLuint getProgram();
        const std::string getName() const;

        void setTransform(const glm::mat4& transform);

        template<class VAL_TYPE>
        boost::shared_ptr<GLShaderParamTemplate<VAL_TYPE> > getParam(
                const std::string& sName)
        {
            unsigned pos;
            bool bFound = findParam(sName, pos);
            GLShaderParamPtr pParam;
            if (bFound) {
                pParam = m_pParams[pos];
            } else {
                pParam = GLShaderParamPtr(
                        new GLShaderParamTemplate<VAL_TYPE>(this, sName));
                m_pParams.insert(m_pParams.begin()+pos, pParam);
            }
            return boost::dynamic_pointer_cast<
                    GLShaderParamTemplate<VAL_TYPE> >(pParam);
        }

    private:
        OGLShader(const std::string& sName, const std::string& sVertProgram, 
                const std::string& sFragProgram, const std::string& sVertPrefix, 
                const std::string& sFragPrefix);
        friend class ShaderRegistry;

        GLuint compileShader(GLenum shaderType, const std::string& sProgram,
                const std::string& sPrefix);
        bool findParam(const std::string& sName, unsigned& pos);
        void dumpInfoLog(GLuint hObj, long severity, bool bIsProgram=false);
        std::string removeATIInfoLogSpam(const std::string& sLog);

        std::string m_sName;
        GLuint m_hVertexShader;
        GLuint m_hFragmentShader;
        GLuint m_hProgram;
        std::string m_sVertProgram;
        std::string m_sFragProgram;

        std::vector<GLShaderParamPtr> m_pParams;
        Mat4fGLShaderParamPtr m_pTransformParam;

        // Dumb pointer for speed reasons.
        ShaderRegistry* m_pShaderRegistry;
};

typedef boost::shared_ptr<OGLShader> OGLShaderPtr;

}

#endif

