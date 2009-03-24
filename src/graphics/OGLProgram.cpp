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

#include "OGLProgram.h"

#include "../base/Logger.h"
#include "../base/Exception.h"

#include "boost/tuple/tuple.hpp"
#include "boost/tuple/tuple_comparison.hpp"

#include <map>
#include <iostream>


namespace avg {

using namespace std;
typedef boost::tuple<OGLShaderPtr, OGLShaderPtr> ShaderPair; 
typedef map<ShaderPair, OGLProgramPtr> ProgramCacheType;
static ProgramCacheType ProgramCache;


void OGLProgram::flushCache()
{
    ProgramCache.clear();
}

OGLProgramPtr OGLProgram::buildProgram(OGLShaderPtr fragmentShader)
{
    return OGLProgram::buildProgram( fragmentShader, OGLShaderPtr());
}

OGLProgramPtr OGLProgram::buildProgram(OGLShaderPtr fragmentShader, OGLShaderPtr vertexShader)
{
    ProgramCacheType::iterator pos;
    OGLProgramPtr prog;
    pos = ProgramCache.find( ShaderPair(fragmentShader,vertexShader) );
    if (pos != ProgramCache.end()){
        //Found it!
        prog = pos->second;
    } else {
        prog = OGLProgramPtr(new OGLProgram(fragmentShader, vertexShader));
        ProgramCache[ShaderPair(fragmentShader, vertexShader)] = prog;
        //cerr<<"Build Shader Program: "<<fragmentShader<<", "<<vertexShader<<endl;
    }
    return prog;
}

void OGLProgram::attachAndLink()
{
    int count = 0;
    for(vector<OGLShaderPtr>::iterator it=m_vShaders.begin(); it!=m_vShaders.end(); ++it) {
        if (*it) {
            glproc::AttachObject(m_hProgram, (*it)->getGLHandle());
            ++count;
        }
    }
    if (count == 0) {
        //No Programs ...
        m_hProgram = 0;
        return;
    }
    glproc::LinkProgram(m_hProgram);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "OGLProgram::OGLProgram: glLinkProgram()");

    GLint bLinked;
    glproc::GetObjectParameteriv(m_hProgram, GL_OBJECT_LINK_STATUS_ARB, &bLinked);
    dumpInfoLog(m_hProgram);
    if (!bLinked) {
        throw(Exception(AVG_ERR_VIDEO_GENERAL,"Linking shader programs failed. Aborting."));
    }
}
OGLProgram::OGLProgram(OGLShaderPtr pFragmentShader)
{
    m_hProgram = glproc::CreateProgramObject();
    m_vShaders.push_back(pFragmentShader);
    attachAndLink();
}

OGLProgram::OGLProgram(OGLShaderPtr pFragmentShader, OGLShaderPtr pVertexShader)
{
    m_hProgram = glproc::CreateProgramObject();
    m_vShaders.push_back(pFragmentShader);
    m_vShaders.push_back(pVertexShader);
    attachAndLink();
}

OGLProgram::OGLProgram(vector<OGLShaderPtr> &vShaders)
{
    m_hProgram = glproc::CreateProgramObject();
    for(vector<OGLShaderPtr>::iterator it=vShaders.begin(); it!=vShaders.end(); ++it) {
        m_vShaders.push_back(*it);
    }
    attachAndLink();
}

OGLProgram::~OGLProgram()
{
    //TODO: Fix shader lifetime in GPUXxxFilter.
    //glproc::DeleteProgram(m_hProgram); 
    //FIXME Deleteing the GLProgram breaks SDLDisplayEngine::render somehow???
}

void OGLProgram::activate()
{
   glproc::UseProgramObject(m_hProgram);
   OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "OGLProgram::activate: glUseProgramObject()");
}

void OGLProgram::deactivate()
{
   glproc::UseProgramObject(0);
   OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "OGLProgram::deactivate: glUseProgramObject()");
}
GLhandleARB OGLProgram::getProgram()
{
    return m_hProgram;
}

void OGLProgram::setUniformIntParam(const std::string& sName, int val)
{
    int loc = safeGetUniformLoc(sName);
    glproc::Uniform1i(loc, val);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, (string("OGLProgram: glUniform(")+sName+")").c_str());
}

void OGLProgram::setUniformFloatParam(const std::string& sName, float val)
{
    int loc = safeGetUniformLoc(sName);
    glproc::Uniform1f(loc, val);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, (string("OGLProgram: glUniform(")+sName+")").c_str());
}

void OGLProgram::setUniformFloatArrayParam(const std::string& sName, int count, float* pVal)
{
    int loc = safeGetUniformLoc(sName);
    glproc::Uniform1fv(loc, count, pVal);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, (string("OGLProgram: glUniform(")+sName+")").c_str());
}
        

int OGLProgram::safeGetUniformLoc(const std::string& sName)
{
    int loc = glproc::GetUniformLocation(m_hProgram, sName.c_str());
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            "OGLProgram::setUniformIntParam: GetUniformLocation()");
    return loc;
}

}
