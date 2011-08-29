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
#include "../base/Logger.h"
#include "../base/Exception.h"

using namespace std;
using namespace boost;

namespace avg {
    
thread_specific_ptr<ShaderRegistryPtr> ShaderRegistry::s_pInstance;

ShaderRegistryPtr& ShaderRegistry::get() 
{
    if (s_pInstance.get() == 0) {
        s_pInstance.reset(new ShaderRegistryPtr(new ShaderRegistry()));
    }
    return *s_pInstance;
}

void ShaderRegistry::kill()
{
    s_pInstance.reset();
}

ShaderRegistry::ShaderRegistry()
{
}

ShaderRegistry::~ShaderRegistry() 
{
}

OGLShaderPtr ShaderRegistry::getOrCreateShader(const std::string& sID, 
        const std::string& sProgram)
{
    OGLShaderPtr pShader = getShader(sID);
    if (!pShader) {
        m_ShaderMap[sID] = OGLShaderPtr(new OGLShader(sProgram));
    }
    return pShader;
}

OGLShaderPtr ShaderRegistry::getShader(const std::string& sID)
{
    ShaderMap::iterator it = m_ShaderMap.find(sID);
    if (it == m_ShaderMap.end()) {
        return OGLShaderPtr();
    } else {
        return it->second;
    }
}

OGLShaderPtr getOrCreateShader(const std::string& sID, const std::string& sProgram)
{
    return ShaderRegistry::get()->getOrCreateShader(sID, sProgram);
}

OGLShaderPtr getShader(const std::string& sID)
{
    return ShaderRegistry::get()->getShader(sID);
}

}

