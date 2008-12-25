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
//  Original author of this file is Jan Boelsche (regular.gonzales@googlemail.com).
//

#include "PluginManager.h"

#include "NodeDefinition.h"
#include "Player.h"

#include "../base/FileHelper.h"
#include "../base/Logger.h"
#include "../base/OSHelper.h"
#include "dlfcn.h"

#include <iostream>
#include <string>

using namespace std;
using namespace avg;

#ifdef _WIN32
#define PATH_DELIMITER ";"
#define PLUGIN_EXTENSION ".dll"
#else
#define PATH_DELIMITER ":"
#define PLUGIN_EXTENSION ".so"
#endif

PluginManager::PluginNotFound::PluginNotFound(const string& message) :
    Exception(AVG_ERR_FILEIO, message) {}

PluginManager::PluginCorrupted::PluginCorrupted(const string& message) :
    Exception(AVG_ERR_CORRUPT_PLUGIN, message) {}
    
PluginManager& PluginManager::get()
{
    static PluginManager sTheInstance;
    return sTheInstance;
}

PluginManager::PluginManager()
{
    setSearchPath(string("."PATH_DELIMITER) + "./plugin"PATH_DELIMITER + 
            getAvgLibPath() + "plugin");
}

void PluginManager::setSearchPath(const string& sNewPath)
{
    m_sCurrentSearchPath = sNewPath;
    parsePath(m_sCurrentSearchPath);
}

string PluginManager::getSearchPath() const
{
    return m_sCurrentSearchPath;
}
    
void PluginManager::loadPlugin(const std::string& sPluginName)
{
    // is it leaded aready?
    PluginMap::iterator i = m_LoadedPlugins.find(sPluginName);
    if (i == m_LoadedPlugins.end()) {
        // no, let's try to load it!
        string sFullpath = locateSharedObject(sPluginName+PLUGIN_EXTENSION);
        void *handle = internalLoadPlugin(sFullpath);
        // add to map of loaded plugins
        m_LoadedPlugins[sPluginName] = make_pair(handle, 1);
    } else {
        // yes, just increase the reference count
        int referenceCount = i->second.second;
        ++referenceCount;
        m_LoadedPlugins[sPluginName] = make_pair(i->second.first, referenceCount);
    }
}

string PluginManager::locateSharedObject(const string& sFilename)
{
    vector<string>::iterator i = m_PathComponents.begin();
    string sFullpath;
    while(i != m_PathComponents.end()) {
        sFullpath = *i + sFilename;
        if (fileExists(sFullpath)) {
            return sFullpath;
        }
        ++i;
    }
    string sMessage = "unable to locate plugin file '" + sFilename + "'. Was looking in " + m_sCurrentSearchPath;
    AVG_TRACE(Logger::PLUGIN, sMessage);        
    throw PluginNotFound(sMessage);
}

string PluginManager::checkDirectory(const string& sDirectory)
{
    string sFixedDirectory;
    char lastChar = *sDirectory.rbegin();
    if (lastChar != '/' && lastChar != '\\') {
        sFixedDirectory = sDirectory + "/";
    } else {
        sFixedDirectory = sDirectory;
    }
    return sFixedDirectory;
}

void PluginManager::parsePath(const std::string& sPath)
{
    // break the string into colon separated components
    // and make sure each component has a trailing slash
    // warn about non-existing directories
    
    m_PathComponents.clear();
    string sRemaining = sPath;
    string::size_type i;
    do {
        i = sRemaining.find(PATH_DELIMITER);
        string sDirectory;
        if (i == string::npos) {
            sDirectory = sRemaining;
            sRemaining = "";
        } else {
            sDirectory = sRemaining.substr(0, i);
            sRemaining = sRemaining.substr(i+1);
        }
        sDirectory = checkDirectory(sDirectory);
        AVG_TRACE(Logger::PLUGIN, "adding plugin directory '" << sDirectory << "'");        
        
        m_PathComponents.push_back(sDirectory);
    } while(!sRemaining.empty());
}
    
void* PluginManager::internalLoadPlugin(const string& sFullpath)
{   
    AVG_TRACE(Logger::PLUGIN, "dlopening '" << sFullpath << "'");
    void *handle = dlopen(sFullpath.c_str(), RTLD_LOCAL | RTLD_NOW);
    if (!handle) {
        string sMessage(dlerror());
        AVG_TRACE(Logger::PLUGIN, "dlopen failed with message '" << sMessage << "'");
        throw PluginCorrupted(sMessage);
    }
    try {
        registerPlugin(handle);
    } catch(PluginCorrupted& e) {
        dlclose(handle);
        throw e;
    }    
    return handle;
}

void PluginManager::registerPlugin(void* handle)
{
   typedef void (*RegisterPluginPtr)();
   RegisterPluginPtr registerPlugin = 
       reinterpret_cast<RegisterPluginPtr> (dlsym(handle, "registerPlugin"));

    if (registerPlugin) {
        AVG_TRACE(Logger::PLUGIN, "Plugin registration function detected");
        registerPlugin();
    } else {
        AVG_TRACE(Logger::PLUGIN, "No plugin registration function detected");
        throw PluginCorrupted("No plugin registration function detected");
    }
}

