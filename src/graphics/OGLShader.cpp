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
#include "../base/OSHelper.h"

#include "../graphics/VertexArray.h"

#include <iostream>
#include <sstream>

using namespace std;

namespace avg {

OGLShader::OGLShader(const string& sName, const string& sVertProgram, 
        const string& sFragProgram, const string& sVertPrefix, const string& sFragPrefix)
    : m_sName(sName),
      m_sVertProgram(sVertProgram),
      m_sFragProgram(sFragProgram)
{
    m_hProgram = glproc::CreateProgram();
    if (sVertProgram == "") {
        m_hVertexShader = 0;
    } else {
        glproc::BindAttribLocation(m_hProgram, VertexArray::TEX_INDEX, "a_TexCoord");
        glproc::BindAttribLocation(m_hProgram, VertexArray::COLOR_INDEX, "a_Color");
        glproc::BindAttribLocation(m_hProgram, VertexArray::POS_INDEX, "a_Pos");
        m_hVertexShader = compileShader(GL_VERTEX_SHADER, sVertProgram, sVertPrefix);
        glproc::AttachShader(m_hProgram, m_hVertexShader);
    }
    m_hFragmentShader = compileShader(GL_FRAGMENT_SHADER, sFragProgram, sFragPrefix);
    
    glproc::AttachShader(m_hProgram, m_hFragmentShader);
    glproc::LinkProgram(m_hProgram);
    GLContext::checkError("OGLShader::OGLShader: glLinkProgram()");

    GLint bLinked;
    glproc::GetProgramiv(m_hProgram, GL_LINK_STATUS, &bLinked);
    if (!bLinked) {
        AVG_LOG_ERROR("Linking shader program '"+sName+"' failed. Aborting.");
        dumpInfoLog(m_hVertexShader, Logger::severity::ERROR);
        dumpInfoLog(m_hFragmentShader, Logger::severity::ERROR);
        dumpInfoLog(m_hProgram, Logger::severity::ERROR, true);
        exit(-1);
    } else {
        AVG_TRACE(Logger::category::SHADER, Logger::severity::INFO,
                "Linking shader program '"+sName+"'.");
        dumpInfoLog(m_hVertexShader, Logger::severity::INFO);
        dumpInfoLog(m_hFragmentShader, Logger::severity::INFO);
        dumpInfoLog(m_hProgram, Logger::severity::INFO, true);
    }
    m_pShaderRegistry = &*ShaderRegistry::get();
    if (m_hVertexShader) {
        m_pTransformParam = getParam<glm::mat4>("transform");
    }
}

OGLShader::~OGLShader()
{
}

bool isMountainLion()
{
#ifdef __APPLE__
    return getOSXMajorVersion() == 12;
#else
    return false;
#endif
}

void OGLShader::activate()
{
    // If we're running on OS X mountain lion, we need to disable shader activation 
    // caching (See bug #355).
    OGLShaderPtr pCurShader = m_pShaderRegistry->getCurShader();
    if (isMountainLion() || !pCurShader || &*pCurShader != this) {
        glproc::UseProgram(m_hProgram);
        m_pShaderRegistry->setCurShader(m_sName);
        GLContext::checkError("OGLShader::activate: glUseProgram()");
    }
}

GLuint OGLShader::getProgram()
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
#ifdef AVG_ENABLE_EGL
        // No fixed-function vertex shader in gles
        AVG_ASSERT(false);
#else
        glLoadMatrixf(glm::value_ptr(transform));
#endif
    }
}

GLuint OGLShader::compileShader(GLenum shaderType, const std::string& sProgram,
        const std::string& sPrefix)
{
    const char * pProgramStrs[2];
    pProgramStrs[0] = sPrefix.c_str();
    pProgramStrs[1] = sProgram.c_str();
    GLuint hShader = glproc::CreateShader(shaderType);
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

void OGLShader::dumpInfoLog(GLuint hObj, long level, bool bIsProgram)
{
    int infoLogLength;
    GLchar * pInfoLog;

    if (!hObj) {
        return;
    }

    if (bIsProgram) {
        glproc::GetProgramiv(hObj, GL_INFO_LOG_LENGTH, &infoLogLength);
    } else {
        glproc::GetShaderiv(hObj, GL_INFO_LOG_LENGTH, &infoLogLength);
    }
    GLContext::checkError("OGLShader::dumpInfoLog: glGetShaderiv()");
    if (infoLogLength > 1) {
        pInfoLog = (GLchar*)malloc(infoLogLength);
        int charsWritten;
        if (bIsProgram) {
            glproc::GetProgramInfoLog(hObj, infoLogLength, &charsWritten, pInfoLog);
        } else {
            glproc::GetShaderInfoLog(hObj, infoLogLength, &charsWritten, pInfoLog);
        }
        string sLog = removeATIInfoLogSpam(pInfoLog);
        GLContext::checkError("OGLShader::dumpInfoLog: glGetShaderInfoLog()");
        if (sLog.size() > 3) {
            AVG_TRACE(Logger::category::SHADER, level, sLog);
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
