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

#include "OGLShader.h"
#include "ShaderRegistry.h"

#include "../base/Logger.h"
#include "../base/Exception.h"

#include "../graphics/VertexArray.h"

#include <iostream>

using namespace std;

namespace avg {

OGLShader::OGLShader(const string& sName, const string& sVertProgram, 
        const string& sFragProgram, const string& sPrefix)
    : m_sName(sName),
      m_sVertProgram(sVertProgram),
      m_sFragProgram(sFragProgram)
{
    m_hProgram = glproc::CreateProgramObject();
    if (sVertProgram == "") {
        m_hVertexShader = 0;
    } else {
        glproc::BindAttribLocation(m_hProgram, VertexArray::TEX_INDEX, "a_TexCoord");
        glproc::BindAttribLocation(m_hProgram, VertexArray::COLOR_INDEX, "a_Color");
        glproc::BindAttribLocation(m_hProgram, VertexArray::POS_INDEX, "a_Pos");
        m_hVertexShader = compileShader(GL_VERTEX_SHADER, sVertProgram, sPrefix);
        glproc::AttachObject(m_hProgram, m_hVertexShader);
    }
    m_hFragmentShader = compileShader(GL_FRAGMENT_SHADER, sFragProgram, sPrefix);
    
    glproc::AttachObject(m_hProgram, m_hFragmentShader);
    glproc::LinkProgram(m_hProgram);
    GLContext::checkError("OGLShader::OGLShader: glLinkProgram()");

    GLint bLinked;
    glproc::GetObjectParameteriv(m_hProgram, GL_OBJECT_LINK_STATUS_ARB, &bLinked);
    if (!bLinked) {
        AVG_TRACE(Logger::ERROR, "Linking shader program '"+sName+"' failed. Aborting.");
        dumpInfoLog(m_hVertexShader, Logger::ERROR);
        dumpInfoLog(m_hFragmentShader, Logger::ERROR);
        dumpInfoLog(m_hProgram, Logger::ERROR);
        exit(-1);
    } else {
        AVG_TRACE(Logger::SHADER, "Linking shader program '"+sName+"'.");
        dumpInfoLog(m_hVertexShader, Logger::SHADER);
        dumpInfoLog(m_hFragmentShader, Logger::SHADER);
        dumpInfoLog(m_hProgram, Logger::SHADER);
    }
    m_pShaderRegistry = ShaderRegistry::get();
    if (m_hVertexShader) {
        m_pTransformParam = getParam<glm::mat4>("transform");
    }
}

OGLShader::~OGLShader()
{
}

void OGLShader::activate()
{
    OGLShaderPtr pCurShader = m_pShaderRegistry->getCurShader();
    if (!pCurShader || &*pCurShader != this) {
        glproc::UseProgramObject(m_hProgram);
        m_pShaderRegistry->setCurShader(m_sName);
        GLContext::checkError("OGLShader::activate: glUseProgramObject()");
    }
}

GLhandleARB OGLShader::getProgram()
{
    return m_hProgram;
}

const std::string OGLShader::getName() const
{
    return m_sName;
}

void OGLShader::setTransform(const glm::mat4& transform)
{
    if (m_hVertexShader) {
        m_pTransformParam->set(transform);
    } else {
        glLoadMatrixf(glm::value_ptr(transform));
    }
}

GLhandleARB OGLShader::compileShader(GLenum shaderType, const std::string& sProgram,
        const std::string& sPrefix)
{
    const char * pProgramStrs[2];
    pProgramStrs[0] = sPrefix.c_str();
    pProgramStrs[1] = sProgram.c_str();
    GLhandleARB hShader = glproc::CreateShaderObject(shaderType);
    glproc::ShaderSource(hShader, 2, pProgramStrs, 0);
    glproc::CompileShader(hShader);
    GLContext::checkError("OGLShader::compileShader()");
    return hShader;
}

bool OGLShader::findParam(const std::string& sName, unsigned& pos)
{
    GLShaderParamPtr pParam;
    bool bFound = false;
    pos = 0;
    while (!bFound && pos<m_pParams.size() && m_pParams[pos]->getName() <= sName) {
        if (m_pParams[pos]->getName() == sName) {
            bFound = true;
        } else {
            ++pos;
        }
    }
    return bFound;
}

void OGLShader::dumpInfoLog(GLhandleARB hObj, int category)
{
    int infoLogLength;
    GLcharARB * pInfoLog;

    if (!hObj) {
        return;
    }

    glproc::GetObjectParameteriv(hObj, GL_OBJECT_INFO_LOG_LENGTH_ARB, &infoLogLength);
    GLContext::checkError("OGLShader::dumpInfoLog: glGetObjectParameteriv()");
    if (infoLogLength > 1) {
        pInfoLog = (GLcharARB*)malloc(infoLogLength);
        int charsWritten;
        glproc::GetInfoLog(hObj, infoLogLength, &charsWritten, pInfoLog);
        string sLog = removeATIInfoLogSpam(pInfoLog);
        GLContext::checkError("OGLShader::dumpInfoLog: glGetInfoLog()");
        if (sLog.size() > 3) {
            AVG_TRACE(category, sLog);
        }
        free(pInfoLog);
    }
}

string OGLShader::removeATIInfoLogSpam(const string& sOrigLog)
{
    istringstream stream(sOrigLog);
    string sLog;
    string sCurLine;
    while(getline(stream, sCurLine)) {
        bool bLineBroken = (sCurLine.find(
                "shader was successfully compiled to run on hardware.") != string::npos)
                || (sCurLine.find("shader(s) linked.") != string::npos);
        if (!bLineBroken) {
            sLog.append(sCurLine+"\n");
        }
    }
    return sLog;
}

}
