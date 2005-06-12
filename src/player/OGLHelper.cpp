//
// $Id$
// 

#include "OGLHelper.h"

#include "../base/Logger.h"
#include "../base/Exception.h"

#include "GL/gl.h"
#include "GL/glu.h"

#include <iostream>
#include <sstream>

using namespace std;

namespace avg {

void OGLErrorCheck(int avgcode, string where) {
    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        stringstream s;
        s << "OpenGL error in " << where <<": " << gluErrorString(err) 
            << " (#" << err << ") ";
        AVG_TRACE(Logger::ERROR, s.str());
        if (err != GL_INVALID_OPERATION) {
            OGLErrorCheck(avgcode, "  --");
        }
        throw Exception(avgcode, s.str());
    }
}

bool queryOGLExtension(char *extName)
{
    /*
    ** Search for extName in the extensions string. Use of strstr()
    ** is not sufficient because extension names can be prefixes of
    ** other extension names. Could use strtok() but the constant
    ** string returned by glGetString might be in read-only memory.
    */
    char *p;
    char *end;
    int extNameLen;

    extNameLen = strlen(extName);

    p = (char *)glGetString(GL_EXTENSIONS);
//    cout << "OpenGL extensions string: " << p << endl;
    if (NULL == p) {
        return false;
    }

    end = p + strlen(p);

    while (p < end) {
        int n = strcspn(p, " ");
        if ((extNameLen == n) && (strncmp(extName, p, n) == 0)) {
            return true;
        }
        p += (n + 1);
    }
    return false;
}

}
