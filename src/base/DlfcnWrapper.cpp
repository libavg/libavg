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

#include "DlfcnWrapper.h"

#include <sstream>
#include <string.h>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include "windows.h"

using namespace std;

namespace avg {

void* dlopen(const char *pszPath, int ignored)
{
    return LoadLibrary(pszPath);
}

void dlclose(void* handle)
{
    FreeLibrary((HMODULE)handle);
}

void* dlsym(void* handle, const char* functionName)
{
    return GetProcAddress((HMODULE)handle, functionName);
}

const char* dlerror()
{
    static char buffer[1024];
    int err = GetLastError();
    ostringstream ss;
    ss << err;
    strncpy(buffer,ss.str().c_str(),1023);
    return buffer;
}

}
