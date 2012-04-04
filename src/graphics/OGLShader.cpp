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

#include <iostream>

using namespace std;

namespace avg {

OGLShader::OGLShader(string sName, string sProgram)
    : m_sName(sName),
      m_sProgram(sProgram)
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
        AVG_TRACE(Logger::ERROR, "Linking shader program '"+sName+"' failed. Aborting.");
        exit(-1);
    }
    
}

OGLShader::~OGLShader()
{
}

void OGLShader::activate()
{
    OGLShaderPtr pCurShader = ShaderRegistry::get()->getCurShader();
    if (!pCurShader || &*pCurShader != this) {
        glproc::UseProgramObject(m_hProgram);
        ShaderRegistry::get()->setCurShader(m_sName);
        OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "OGLShader::activate: glUseProgramObject()");
    }
}

void OGLShader::deactivate()
{
    OGLShaderPtr pCurShader = ShaderRegistry::get()->getCurShader();
    if (pCurShader) {
        glproc::UseProgramObject(0);
        ShaderRegistry::get()->setCurShader("");
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
        string sLog = removeATIInfoLogSpam(pInfoLog);
        OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "OGLShader::dumpInfoLog: glGetInfoLog()");
        AVG_TRACE(Logger::WARNING, sLog);
        free(pInfoLog);
    }
}

string OGLShader::removeATIInfoLogSpam(const string& sOrigLog)
{
    istringstream stream(sOrigLog);
    string sLog;
    string sCurLine;
    while(getline(stream, sCurLine)) {
        if ((sCurLine.find(
                "Fragment shader was successfully compiled to run on hardware.")
                == string::npos) &&
                (sCurLine.find("Fragment shader(s) linked.") == string::npos))
        {
            sLog.append(sCurLine+"\n");
        }
    }
    return sLog;
}

}
