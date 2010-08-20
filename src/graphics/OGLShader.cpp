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
#include "../base/Matrix3x4.h"

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
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            (string("OGLShader: glUniform(")+sName+")").c_str());
}

void OGLShader::setUniformFloatParam(const std::string& sName, float val)
{
    int loc = safeGetUniformLoc(sName);
    glproc::Uniform1f(loc, val);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            (string("OGLShader: glUniform(")+sName+")").c_str());
}

void OGLShader::setUniformFloatArrayParam(const std::string& sName, int count, 
        float* pVal)
{
    int loc = safeGetUniformLoc(sName);
    glproc::Uniform1fv(loc, count, pVal);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            (string("OGLShader: glUniform(")+sName+")").c_str());
}

void OGLShader::setUniformDPointParam(const std::string& sName, DPoint pt)
{
    int loc = safeGetUniformLoc(sName);
    glproc::Uniform2f(loc, (float)pt.x, (float)pt.y);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            (string("OGLShader: glUniform(")+sName+")").c_str());
}
        
void OGLShader::setUniformColorParam(const std::string& sName, Pixel32 col)
{
    int loc = safeGetUniformLoc(sName);
    glproc::Uniform4f(loc, col.getR()/255.f, col.getG()/255.f, col.getB()/255.f,
            col.getA()/255.f);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            (string("OGLShader: glUniform(")+sName+")").c_str());
}
        
void OGLShader::setUniformVec4fParam(const std::string& sName, float x, float y, float z, 
                float w)
{
    int loc = safeGetUniformLoc(sName);
    glproc::Uniform4f(loc, x, y, z, w);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            (string("OGLShader: glUniform(")+sName+")").c_str());
}

void OGLShader::setUniformMatrix3x4Param(const std::string& sName, const Matrix3x4& mat)
{
    int loc = safeGetUniformLoc(sName);
    GLfloat glMat[4][4];
    for (int x=0; x<3; ++x) {
        for (int y=0; y<4; ++y) {
            glMat[x][y] = mat.val[x][y];
        }
    }
    for (int y=0; y<4; ++y) {
        glMat[3][y] = 0.0f;
    }

    glproc::UniformMatrix4fv(loc, 1, GL_TRUE, (GLfloat *)glMat);
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
