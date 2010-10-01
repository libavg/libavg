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

#ifndef _OGLShader_H_
#define _OGLShader_H_

#include "../api.h"
#include "OGLHelper.h"
#include "Pixel32.h"

#include "../base/Point.h"

#include <boost/shared_ptr.hpp>

#include <string>

namespace avg {

class Matrix3x4;

class AVG_API OGLShader {
    public:
        virtual ~OGLShader();

        void activate();
        GLhandleARB getProgram();

        void setUniformIntParam(const std::string& sName, int val);
        void setUniformFloatParam(const std::string& sName, float val);
        void setUniformFloatParam(const std::string& sName, double val);
        void setUniformFloatArrayParam(const std::string& sName, int count, float* pVal);
        void setUniformDPointParam(const std::string& sName, DPoint pt);
        void setUniformColorParam(const std::string& sName, Pixel32 col);
        void setUniformVec4fParam(const std::string& sName, float x, float y, float z, 
                float w);
        void setUniformMatrix3x4Param(const std::string& sName, const Matrix3x4& mat);

    private:
        OGLShader(std::string sProgram);
        friend class ShaderRegistry;

        void dumpInfoLog(GLhandleARB hObj);
        int safeGetUniformLoc(const std::string& sName);

        GLhandleARB m_hFragmentShader;
        GLhandleARB m_hProgram;
        std::string m_sProgram;
};

typedef boost::shared_ptr<OGLShader> OGLShaderPtr;

}

#endif

