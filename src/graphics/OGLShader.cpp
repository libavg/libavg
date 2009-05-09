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

#include "../base/Logger.h"
#include "../base/Exception.h"
#include <iostream>

using namespace std;

namespace avg {

OGLShader::OGLShader(string sProgram)
    : m_sProgram(sProgram)
{
    m_hFragmentShader = glproc::CreateShaderObject(GL_FRAGMENT_SHADER);
    const char * pProgramStr = m_sProgram.c_str();
    glproc::ShaderSource(m_hFragmentShader, 1, &pProgramStr, 0);
    glproc::CompileShader(m_hFragmentShader);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "OGLShader::OGLShader: glCompileShader()");
    dumpInfoLog(m_hFragmentShader);

    m_hProgram = glproc::CreateProgramObject();
    glproc::AttachObject(m_hProgram, m_hFragmentShader);
    glproc::LinkProgram(m_hProgram);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "OGLShader::OGLShader: glLinkProgram()");

    GLint bLinked;
    glproc::GetObjectParameteriv(m_hProgram, GL_OBJECT_LINK_STATUS_ARB, &bLinked);
    dumpInfoLog(m_hProgram);
    if (!bLinked) {
        AVG_TRACE(Logger::ERROR, "Linking shader program failed. Aborting.");
        exit(-1);
    }
    
}

OGLShader::~OGLShader()
{
}

void OGLShader::activate()
{
   glproc::UseProgramObject(m_hProgram);
   OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "OGLShader::activate: glUseProgramObject()");
}

GLhandleARB OGLShader::getProgram()
{
    return m_hProgram;
}

void OGLShader::setUniformIntParam(const std::string& sName, int val)
{
    int loc = safeGetUniformLoc(sName);
    glproc::Uniform1i(loc, val);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, (string("OGLShader: glUniform(")+sName+")").c_str());
}

void OGLShader::setUniformFloatParam(const std::string& sName, float val)
{
    int loc = safeGetUniformLoc(sName);
    glproc::Uniform1f(loc, val);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, (string("OGLShader: glUniform(")+sName+")").c_str());
}

void OGLShader::setUniformFloatArrayParam(const std::string& sName, int count, float* pVal)
{
    int loc = safeGetUniformLoc(sName);
    glproc::Uniform1fv(loc, count, pVal);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, (string("OGLShader: glUniform(")+sName+")").c_str());
}
        
void OGLShader::dumpInfoLog(GLhandleARB hObj)
{
    int InfoLogLength;
    GLcharARB * pInfoLog;

    glproc::GetObjectParameteriv(hObj, GL_OBJECT_INFO_LOG_LENGTH_ARB, &InfoLogLength);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            "OGLShader::dumpInfoLog: glGetObjectParameteriv()");
    if (InfoLogLength > 1) {
        pInfoLog = (GLcharARB*)malloc(InfoLogLength);
        int CharsWritten;
        glproc::GetInfoLog(hObj, InfoLogLength, &CharsWritten, pInfoLog);
        OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
                "OGLShader::dumpInfoLog: glGetInfoLog()");
        AVG_TRACE(Logger::WARNING, pInfoLog);
        free(pInfoLog);
    }
}

int OGLShader::safeGetUniformLoc(const std::string& sName)
{
    int loc = glproc::GetUniformLocation(m_hProgram, sName.c_str());
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            "OGLShader::setUniformIntParam: GetUniformLocation()");
    return loc;
}
 

}
