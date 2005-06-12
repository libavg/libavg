//
// $Id$
// 

#ifndef _OGLHelper_H_
#define _OGLHelper_H_

#include <string>

namespace avg {

void OGLErrorCheck(int avgcode, std::string where);
bool queryOGLExtension(char *extName);

}

// This should be in a system-wide gl header, but for some reason it isn't
// always there...
#ifndef GL_TEXTURE_RECTANGLE_NV
#define GL_TEXTURE_RECTANGLE_NV           0x84F5
#endif

#endif
 
