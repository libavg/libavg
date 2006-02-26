//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2006 Ulrich von Zadow
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

#ifndef _OGLHelper_H_
#define _OGLHelper_H_

#include <string>

namespace avg {

void OGLErrorCheck(int avgcode, std::string where);
bool queryOGLExtension(char *extName);
bool queryGLXExtension(char *extName);

enum OGLMemoryMode { 
    OGL,  // Standard OpenGL
    MESA,
    PBO   // pixel buffer objects
};
        

typedef void (*GLfunction)();
GLfunction getFuzzyProcAddress(const char * psz);

}

// This should be in a system-wide gl header, but for some reason it isn't
// always there...
#ifndef GL_TEXTURE_RECTANGLE_NV
#define GL_TEXTURE_RECTANGLE_NV           0x84F5
#endif

#endif
 
