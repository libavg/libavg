
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

#include "OGLShader.h"

#include "../base/Exception.h"
#include <iostream>

using namespace std;

namespace avg {
    OGLShader::OGLShader(string sProgram, int type):
        m_sProgram(sProgram) 
    {
        if (sProgram==""){
            m_hShader = 0;
            return;

        }
        m_hShader = glproc::CreateShaderObject(type); //Maybe actively check for valid type first
        //cerr<<"Created a GLHandle "<<m_hShader<<" in "<<this<<endl;
        const char * pProgramStr = m_sProgram.c_str();
        glproc::ShaderSource(m_hShader, 1, &pProgramStr, 0);
        glproc::CompileShader(m_hShader);
        OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "OGLShader::OGLShader: glCompileShader()");
        dumpInfoLog(m_hShader);
    }
    OGLShader::~OGLShader() {
        //cerr<<"Deleting a GLHandle "<<m_hShader<<" in "<<this<<endl;
        glproc::DeleteShader(m_hShader);
        OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "OGLshader::OGLShader: Destructor");
        m_hShader = -1;
    }

    int OGLShader::getType() {
        return m_Type;
    }
    GLhandleARB OGLShader::getGLHandle() {
        return m_hShader;
    }

}
