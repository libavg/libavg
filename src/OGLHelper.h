//
// $Id$
// 

#ifndef _OGLHelper_H_
#define _OGLHelper_H_

#include <string>

void OGLErrorCheck(int avgcode, std::string where);
bool queryOGLExtension(char *extName);

#endif
 
