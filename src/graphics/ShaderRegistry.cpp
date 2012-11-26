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

#include "ShaderRegistry.h"

#include "GLContext.h"
#include "OGLShader.h"

#include "../base/Logger.h"
#include "../base/Exception.h"
#include "../base/OSHelper.h"
#include "../base/FileHelper.h"
#include "../base/StringHelper.h"

#include <iostream>

using namespace std;
using namespace boost;

namespace avg {

std::string ShaderRegistry::s_sLibPath;
    
ShaderRegistryPtr ShaderRegistry::get() 
{
    return GLContext::getCurrent()->getShaderRegistry();
}

ShaderRegistry::ShaderRegistry()
{
    if (s_sLibPath == "") {
        string sShaderPath = getPath(getAvgLibPath())+"shaders";
#ifdef __linux
        // XXX: Hack to make sure shaders are found during make distcheck under linux. 
        char * pszSrcDir = getenv("srcdir");
        if (pszSrcDir) {
            sShaderPath =  string(pszSrcDir) + "/../graphics/shaders";
        }
#endif
        setShaderPath(sShaderPath);
    }
}

ShaderRegistry::~ShaderRegistry() 
{
}
    
std::string ShaderRegistry::getShaderPath()
{
    return s_sLibPath;
}

void ShaderRegistry::setShaderPath(const string& sLibPath)
{
    if (sLibPath != "") {
        s_sLibPath = sLibPath;
        AVG_TRACE(Logger::CONFIG, "Loading shaders from "+s_sLibPath);
    }
}

void ShaderRegistry::setPreprocessorDefine(const std::string& sName, 
        const std::string& sValue)
{
    m_PreprocessorDefinesMap[sName] = sValue;
}

void ShaderRegistry::createShader(const std::string& sID)
{
    OGLShaderPtr pShader = getShader(sID);
    if (!pShader) {
        string sShaderCode;
        string sFilename = s_sLibPath+"/"+sID+".frag";
        string sVertPreprocessed;
        if (GLContext::getCurrent()->getShaderUsage() != GLConfig::FRAGMENT_ONLY) {
            loadShaderString(s_sLibPath+"/standard.vert", sVertPreprocessed);
        }
        string sFragPreprocessed;
        loadShaderString(sFilename, sFragPreprocessed);
        string sVertPrefix = createPrefixString(false);
        string sFragPrefix = createPrefixString(true);
        m_ShaderMap[sID] = OGLShaderPtr(
                new OGLShader(sID, sVertPreprocessed, sFragPreprocessed, sVertPrefix,
                        sFragPrefix));
    }
}

OGLShaderPtr ShaderRegistry::getShader(const std::string& sID) const
{
    ShaderMap::const_iterator it = m_ShaderMap.find(sID);
    if (it == m_ShaderMap.end()) {
        return OGLShaderPtr();
    } else {
        return it->second;
    }
}

OGLShaderPtr ShaderRegistry::getCurShader() const
{
    return m_pCurShader;
}

void ShaderRegistry::setCurShader(const std::string& sID)
{
    if (sID == "") {
        m_pCurShader = OGLShaderPtr();
    } else {
        m_pCurShader = getShader(sID);
    }
}

void ShaderRegistry::loadShaderString(const string& sFilename, string& sPreprocessed)
{
    string sShaderCode;
    readWholeFile(sFilename, sShaderCode);
    preprocess(sShaderCode, sFilename, sPreprocessed);
}

void ShaderRegistry::preprocess(const string& sShaderCode, const string& sFileName, 
        string& sProcessed)
{
    sProcessed.append("#line 0\n");
    istringstream stream(sShaderCode);
    string sCurLine;
    int curLine = 0;
    while(getline(stream, sCurLine)) {
        curLine++;
        string sStripped = removeStartEndSpaces(sCurLine);
        if (sStripped.substr(0, 8) == "#include") {
            size_t startPos = sStripped.find('"');
            size_t endPos = sStripped.find('"', startPos+1);
            if (startPos == string::npos || endPos == string::npos) {
                throwParseError(sFileName, curLine);
            }
            string sIncFileName = sStripped.substr(startPos+1, endPos-startPos-1);
            sIncFileName = s_sLibPath+"/"+sIncFileName;
            string sIncludedFile;
            readWholeFile(sIncFileName, sIncludedFile);
            string sProcessedIncludedFile;
            preprocess(sIncludedFile, sIncFileName, sProcessedIncludedFile);
            sProcessed.append(sProcessedIncludedFile);
            sProcessed.append("#line "+toString(curLine)+"\n");
        } else {
            sProcessed.append(sCurLine+"\n");
        }
    }
}

string ShaderRegistry::createPrefixString(bool bFragment)
{
    stringstream ss;
    std::map<std::string, std::string>::iterator it;
    for (it = m_PreprocessorDefinesMap.begin(); it != m_PreprocessorDefinesMap.end();
            ++it)
    {
        ss << "#define " << it->first << " " << it->second << endl;
    }
    if (GLContext::getCurrent()->isGLES()) {
        ss << endl;
        if (bFragment) {
            ss << "#extension GL_OES_standard_derivatives : enable" << endl;
        }
        ss << "precision mediump float;" << endl;
    }
    if (GLContext::getCurrent()->getShaderUsage() == GLConfig::FRAGMENT_ONLY) {
        ss << endl;
        ss << "#define v_TexCoord gl_TexCoord[0].st" << endl;
        ss << "#define v_Color gl_Color" << endl;
    }
    return ss.str();
}

void ShaderRegistry::throwParseError(const string& sFileName, int curLine)
{
    throw Exception(AVG_ERR_VIDEO_GENERAL, "File '"+sFileName+"', Line "+
            toString(curLine)+": Syntax error.");
}

void createShader(const std::string& sID)
{
    return ShaderRegistry::get()->createShader(sID);
}

OGLShaderPtr getShader(const std::string& sID)
{
    return ShaderRegistry::get()->getShader(sID);
}

}

