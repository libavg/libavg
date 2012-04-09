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

#ifndef _ShaderRegistry_H_ 
#define _ShaderRegistry_H_

#include "../api.h"

#include "OGLShader.h"

#include <boost/shared_ptr.hpp>

#include <map>

namespace avg {

class ShaderRegistry;
typedef boost::shared_ptr<ShaderRegistry> ShaderRegistryPtr;

class AVG_API ShaderRegistry {
public:
    static ShaderRegistryPtr get();
    ShaderRegistry();
    virtual ~ShaderRegistry();

    void setShaderPath(const std::string& sLibPath);
    void setPreprocessorDefine(const std::string& sName, const std::string& sValue);

    void createShader(const std::string& sID);
    OGLShaderPtr getShader(const std::string& sID) const;

    OGLShaderPtr getCurShader() const;
    void setCurShader(const std::string& sID);

private:
    void preprocess(const std::string& sShaderCode, const std::string& sFileName, 
            std::string& sProcessed);
    std::string createDefinesString();
    void throwParseError(const std::string& sFileName, int curLine);
    typedef std::map<std::string, OGLShaderPtr> ShaderMap;
    ShaderMap m_ShaderMap;
    OGLShaderPtr m_pCurShader;
    std::map<std::string, std::string> m_PreprocessorDefinesMap;

    static std::string m_sLibPath;
};

void createShader(const std::string& sID);
OGLShaderPtr getShader(const std::string& sID);


}

#endif
