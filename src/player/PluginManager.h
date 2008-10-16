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
#ifndef _PluginManager_h_
#define _PluginManager_h_

#include "../base/Exception.h"

#include <map>
#include <vector>
#include <memory>

namespace avg {
	
class PluginManager {
public:
	class PluginNotFound : public Exception {
	public:
		PluginNotFound(const std::string& message);
	};
	class PluginCorrupted : public Exception {
	public:
		PluginCorrupted(const std::string& message);
	};
	
	static PluginManager& get();
	
	void setSearchPath(const std::string& aNewPath);
	std::string getSearchPath() const;
	
	void loadPlugin(const std::string& aPluginName);
	void unloadPlugin(const std::string& aPluginName);

private:
	PluginManager();	
	
	std::string _checkDirectory(const std::string& aDirectory);
	void _parsePath(const std::string& aPath);
	std::string _locateSharedObject(const std::string& aPluginName);
	void* _internalLoadPlugin(const std::string& aPluginName);
	void _internalUnloadPlugin(void*);
	void _inspectPlugin(void* handle);
	
	
	// maps module names to a pair of handle and reference count
	typedef std::map<std::string, std::pair<void*, int> > PluginMap;
	PluginMap m_loadedPlugins;
	std::vector<std::string> m_pathComponents;
	std::string m_currentSearchPath;	
};
	
}

#endif