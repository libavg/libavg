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

#include "../base/FileHelper.h"
#include "../base/Logger.h"

#include <iostream>
#include <string>
#include <dlfcn.h>

using namespace std;
using namespace avg;

PluginManager::PluginNotFound::PluginNotFound(const string& message) :
	Exception(AVG_ERR_FILEIO, message) {}

PluginManager::PluginCorrupted::PluginCorrupted(const string& message) :
	Exception(AVG_ERR_CORRUPT_PLUGIN, message) {}
	
PluginManager& PluginManager::get() {
	static PluginManager theInstance;
	return theInstance;
}

PluginManager::PluginManager() {
	_parsePath(".");
}

void PluginManager::setSearchPath(const string& aNewPath) {
	m_currentSearchPath = aNewPath;
	_parsePath(m_currentSearchPath);
}

string PluginManager::getSearchPath() const {
	return m_currentSearchPath;
}
	
void PluginManager::loadPlugin(const std::string& aPluginName) {
	// is it leaded aready?
	PluginMap::iterator i = m_loadedPlugins.find(aPluginName);
	if (i == m_loadedPlugins.end()) {
		// no, let's try to load it!
		string fullpath = _locateSharedObject(aPluginName+".so");
		void *handle = _internalLoadPlugin(fullpath);
		// add to map of loaded plugins
		m_loadedPlugins[aPluginName] = make_pair(handle, 1);
	} else {
		// yes, just increase the reference count
		int referenceCount = i->second.second;
		++referenceCount;
		m_loadedPlugins[aPluginName] = make_pair(i->second.first, referenceCount);
	}
}

string PluginManager::_locateSharedObject(const string& aFilename) {
	vector<string>::iterator i = m_pathComponents.begin();
	string myFullpath;
	while(i != m_pathComponents.end()) {
		myFullpath = *i + aFilename;
		if (fileExists(myFullpath)) {
			return myFullpath;
		}
		++i;
	}
	string message = "unable to locate plugin file '" + aFilename + "'. Was looking in " + m_currentSearchPath;
	AVG_TRACE(Logger::PLUGIN, message);		
	throw PluginNotFound(message);
}

void PluginManager::unloadPlugin(const string& aPluginName) {
	// is it leaded?
	PluginMap::iterator i = m_loadedPlugins.find(aPluginName);
	if (i == m_loadedPlugins.end()) {
		// no, this is something we probably can ignore
		AVG_TRACE(Logger::PLUGIN, "Warning: request to unload plugin that wasn't loaded: '" << aPluginName << "'.");	
	} else {
		// yes,  decrease the reference count
		int referenceCount = i->second.second;
		if (--referenceCount) {
			// reference count is not zero
			// store the new reference count and keep the module loaded
			m_loadedPlugins[aPluginName] = make_pair(i->second.first, referenceCount);
		} else {
			m_loadedPlugins.erase(i);
			_internalUnloadPlugin(i->second.first);
		}
	}
}

string PluginManager::_checkDirectory(const string& aDirectory) {
	string directory;
	char lastChar = *aDirectory.rbegin();
	if (lastChar != '/' && lastChar != '\\') {
		directory = aDirectory + "/";
	} else {
		directory = aDirectory;
	}
	if (!fileExists(directory)) {
		AVG_TRACE(Logger::PLUGIN, "Warning: non-existing plugin directory '" << directory << "'");		
	}
	return directory;
}

void PluginManager::_parsePath(const std::string& aPath) {
	// break the string into colon separated components
	// and make sure each component has a trailing slash
	// warn about non-existing directories
	
	m_pathComponents.clear();
	string remaining = aPath;
	string::size_type i;
	do {
		i = remaining.find(':');
		string directory;
		if (i == string::npos) {
			directory = remaining;
			remaining = "";
		} else {
			directory = remaining.substr(0, i);
			remaining = remaining.substr(i+1);
		}
		directory = _checkDirectory(directory);
		AVG_TRACE(Logger::PLUGIN, "adding plugin directory '" << directory << "'");		
		
		m_pathComponents.push_back(directory);
	} while(!remaining.empty());
}
	
void* PluginManager::_internalLoadPlugin(const string& fullpath) {	
	AVG_TRACE(Logger::PLUGIN, "dlopening '" << fullpath << "'");
	void *handle = dlopen(fullpath.c_str(), RTLD_LOCAL | RTLD_NOW);
	if (!handle) {
		string message(dlerror());
		AVG_TRACE(Logger::PLUGIN, "dlopen failed with message '" << message << "'");
		throw PluginCorrupted(message);
	}
	try {
		_inspectPlugin(handle);
	} catch(PluginCorrupted& e) {
		_internalUnloadPlugin(handle);
		throw e;
	}
	 
	return handle;
}

void PluginManager::_internalUnloadPlugin(void* handle) {
	AVG_TRACE(Logger::PLUGIN, "dlclosing");
	dlclose(handle);
}

void PluginManager::_inspectPlugin(void* handle) {
	typedef NodeDefinition (*GetNodeDefinitionPtr)();
	GetNodeDefinitionPtr getNodeDefinition = reinterpret_cast<GetNodeDefinitionPtr> (dlsym(handle, "getNodeDefinition"));

	if (getNodeDefinition) {
		AVG_TRACE(Logger::PLUGIN, "NodePlugin detected");
		
		NodeDefinition myNodeDefinition = getNodeDefinition();
		AVG_TRACE(Logger::PLUGIN, "found definition for Node " << myNodeDefinition.getName());
	} else {
		AVG_TRACE(Logger::PLUGIN, "no magic symbols found");
		throw PluginCorrupted("no magic symbols found.");
	}
}

