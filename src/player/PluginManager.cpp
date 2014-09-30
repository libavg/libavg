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

#include "PluginManager.h"

#include "../base/DlfcnWrapper.h"
#include "../base/FileHelper.h"
#include "../base/Logger.h"
#include "../base/OSHelper.h"

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

PluginManager::PluginNotFound::PluginNotFound(const string& sMessage) :
    Exception(AVG_ERR_FILEIO, sMessage) {}

PluginManager::PluginCorrupted::PluginCorrupted(const string& sMessage) :
    Exception(AVG_ERR_CORRUPT_PLUGIN, sMessage) {}
    
PluginManager& PluginManager::get()
{
    static PluginManager s_Instance;
    return s_Instance;
}

PluginManager::PluginManager()
{
    setSearchPath(string("."PATH_DELIMITER) +
            "./plugin"PATH_DELIMITER + 
            "./plugin/.libs"PATH_DELIMITER +
            getPath(getAvgLibPath()) + "plugin");
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
    
py::object PluginManager::loadPlugin(const std::string& sPluginName)
{
    // is it loaded aready?
    PluginMap::iterator i = m_LoadedPlugins.find(sPluginName);
    if (i == m_LoadedPlugins.end()) {
        // no, let's try to load it!
        string sFullpath = locateSharedObject(sPluginName+PLUGIN_EXTENSION);
        void *handle = internalLoadPlugin(sFullpath, sPluginName);
        // add to map of loaded plugins
        m_LoadedPlugins[sPluginName] = make_pair(handle, 1);
    } else {
        // yes, just increase the reference count
        int referenceCount = i->second.second;
        ++referenceCount;
        m_LoadedPlugins[sPluginName] = make_pair(i->second.first, referenceCount);
    }
    py::object sysModule(py::handle<>(PyImport_ImportModule("sys")));
    return sysModule.attr("modules")[sPluginName];
}

string PluginManager::locateSharedObject(const string& sFilename)
{
    vector<string>::iterator i = m_PathComponents.begin();
    string sFullpath;
    while (i != m_PathComponents.end()) {
        sFullpath = *i + sFilename;
        if (fileExists(sFullpath)) {
            return sFullpath;
        }
        ++i;
    }
    string sMessage = "Unable to locate plugin file '" + sFilename 
            + "'. Was looking in " + m_sCurrentSearchPath;
    AVG_TRACE(Logger::category::PLUGIN, Logger::severity::INFO, sMessage);
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

void PluginManager::parsePath(const string& sPath)
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
        
        m_PathComponents.push_back(sDirectory);
    } while (!sRemaining.empty());
    AVG_TRACE(Logger::category::PLUGIN, Logger::severity::INFO,
            "Plugin search path set to '" << sPath << "'"); 
}
    
void* PluginManager::internalLoadPlugin(const string& sFullpath,
        const string& sPluginName)
{   
    void *handle = dlopen(sFullpath.c_str(), RTLD_LOCAL | RTLD_NOW);
    if (!handle) {
        string sMessage(dlerror());
        AVG_TRACE(Logger::category::PLUGIN, Logger::severity::ERROR,
                "Could not load plugin. dlopen failed with message '"
                << sMessage << "'");
        throw PluginCorrupted(sMessage);
    }
    try {
        registerPlugin(handle, sPluginName);
    } catch(PluginCorrupted& e) {
        dlclose(handle);
        throw e;
    }    
    AVG_TRACE(Logger::category::PLUGIN,Logger::severity::INFO,
            "Loaded plugin '" << sFullpath << "'");
    return handle;
}

void PluginManager::registerPlugin(void* handle, const std::string& sPluginName)
{
   typedef PyObject* (*RegisterPluginPtr)();
   RegisterPluginPtr registerPlugin = 
       reinterpret_cast<RegisterPluginPtr>(dlsym(handle, "registerPlugin"));

    if (registerPlugin) {
        PyObject* plugin = registerPlugin();
        py::object sysModule(py::handle<>(PyImport_ImportModule("sys")));
        sysModule.attr("modules")[sPluginName] = py::object(py::handle<>(plugin));

        //TODO: Discuss –– maybe we shouldn't add plugins to the __builtins__ but rather
        //have users ```import PLUGIN```explicitly
        #if PY_MAJOR_VERSION < 3
        PyObject* pyBuiltin = PyImport_AddModule("__builtin__");
        #else
        PyObject* pyBuiltin = PyImport_AddModule("builtins");
        #endif
        int success =  PyModule_AddObject(pyBuiltin, sPluginName.c_str(), plugin);
        AVG_ASSERT(success == 0);

    } else {
        AVG_TRACE(Logger::category::PLUGIN, Logger::severity::ERROR,
                "No plugin registration function detected");
        throw PluginCorrupted("No plugin registration function detected");
    }
}

