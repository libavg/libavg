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
// Original code (c) 2008 Jan Boelsche

#ifndef _IPlugin_h_
#define _IPlugin_h_

#include <string>

namespace avg {

/*	Basic plugin interface.
	To be implemented by node or filter plugins
	
	Reasoning:
	The plugin may use a different heap or allocator for its instance.
	Therefor the interface is designed in a way that ensures that
	new() and delete() and (indirectly) malloc() and free() all are called
	from within the plugin as opposed to the alloc method being called by the 
	plugin and the free method being called by the main module.

	The only method exported by the shared object has a plain-C signature.
	This allows to compile the plugin with a different c++ compiler (or compiler
	version) than the main module.
	This might limit the need for rebuilding older plugin binaries that 
	otherwise would not be ABI compatible anymore.
	(NOTE: Exceptions that cross the plugin border might break this)
*/

struct IPlugin {
	// should return informative string
	virtual std::string getDescription() const {return "n/a";}
	
	// called by the PluginManager to release the instance
	// NOTE: call's the plugin's version of delete (this is a header file)
	virtual void release() {delete this;}
	
	// virtual d'tor
	virtual ~IPlugin() {};
};

}

#ifdef BUILDING_AVG_PLUGIN
extern "C" avg::IPlugin* createPlugin();
#endif

#endif

