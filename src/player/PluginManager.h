//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2014 Ulrich von Zadow
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
//  Original author of this file is Jan Boelsche (regular.gonzales@googlemail.com).
//
#ifndef _PluginManager_H_
#define _PluginManager_H_

#include "../api.h"

#include "BoostPython.h"

#include "../base/Exception.h"

#include <map>
#include <vector>
#include <memory>

namespace avg {
    
class AVG_API PluginManager
{
public:
    class AVG_API PluginNotFound : public Exception {
    public:
        PluginNotFound(const std::string& sMessage);
    };
    class AVG_API PluginCorrupted : public Exception {
    public:
        PluginCorrupted(const std::string& sMessage);
    };
    
    static PluginManager& get();
    
    void setSearchPath(const std::string& aNewPath);
    std::string getSearchPath() const;
    
    py::object loadPlugin(const std::string& aPluginName);

private:
    PluginManager();    
    
    std::string checkDirectory(const std::string& sDirectory);
    void parsePath(const std::string& sPath);
    std::string locateSharedObject(const std::string& sPluginName);
    void* internalLoadPlugin(const std::string& sPluginPath,
            const std::string& sPluginName);
    void registerPlugin(void* pHandle, const std::string& sPluginName);
      
    // maps module names to a pair of handle and reference count
    typedef std::map<std::string, std::pair<void*, int> > PluginMap;
    PluginMap m_LoadedPlugins;
    std::vector<std::string> m_PathComponents;
    std::string m_sCurrentSearchPath;    
};
    
}

#endif

