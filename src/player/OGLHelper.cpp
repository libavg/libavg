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

#include "OGLHelper.h"

#ifdef __APPLE__
#include "AppleGLHelper.h"
#endif

#include "../base/Logger.h"
#include "../base/Exception.h"

#include <SDL/SDL.h>
#include <iostream>
#include <sstream>

using namespace std;

namespace avg {

namespace glproc {
    PFNGLGENBUFFERSPROC GenBuffers;
    PFNGLBUFFERDATAPROC BufferData;
    PFNGLDELETEBUFFERSPROC DeleteBuffers;
    PFNGLBINDBUFFERPROC BindBuffer;
    PFNGLMAPBUFFERPROC MapBuffer;
    PFNGLUNMAPBUFFERPROC UnmapBuffer;
    
    PFNGLCREATESHADEROBJECTARBPROC CreateShaderObject;
    PFNGLSHADERSOURCEARBPROC ShaderSource;
    PFNGLCOMPILESHADERARBPROC CompileShader;
    PFNGLCREATEPROGRAMOBJECTARBPROC CreateProgramObject;
    PFNGLATTACHOBJECTARBPROC AttachObject;
    PFNGLLINKPROGRAMARBPROC LinkProgram;
    PFNGLGETOBJECTPARAMETERIVARBPROC GetObjectParameteriv;
    PFNGLGETINFOLOGARBPROC GetInfoLog;
    PFNGLUSEPROGRAMOBJECTARBPROC UseProgramObject;
    PFNGLGETUNIFORMLOCATIONARBPROC GetUniformLocation;
    PFNGLUNIFORM1IARBPROC Uniform1i;
    PFNGLBLENDEQUATIONPROC BlendEquation;
    PFNGLACTIVETEXTUREPROC ActiveTexture;
#ifdef linux    
    PFNGLXSWAPINTERVALSGIPROC SwapIntervalSGI;
    PFNGLXWAITVIDEOSYNCSGIPROC WaitVideoSyncSGI;
#endif
}    

void OGLErrorCheck(int avgcode, const string & where) {
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
    size_t extNameLen = strlen(extName);

    p = (char *)glGetString(GL_EXTENSIONS);
//    cout << "OpenGL extensions string: " << p << endl;
    if (NULL == p) {
        throw Exception(AVG_ERR_VIDEO_GENERAL, "Couldn't get OpenGL extension string.");
    }

    char * end = p + strlen(p);

    while (p < end) {
        size_t n = strcspn(p, " ");
        if ((extNameLen == n) && (strncmp(extName, p, n) == 0)) {
            return true;
        }
        p += (n + 1);
    }
    return false;
}

bool queryGLXExtension(char *extName) {
#if (defined __APPLE__) || (defined _WIN32)
    return false;
#else
    int extNameLen = strlen(extName);

    Display * display = XOpenDisplay(0);
    char * p = (char *)glXQueryExtensionsString(display, DefaultScreen(display));
    if (NULL == p) {
        throw Exception(AVG_ERR_VIDEO_GENERAL, "Couldn't get GLX extension string.");
    }
//    cout << "GLX extensions string: " << p << endl;

    char * end = p + strlen(p);

    while (p < end) {
        int n = strcspn(p, " ");
        if ((extNameLen == n) && (strncmp(extName, p, n) == 0)) {
            return true;
        }
        p += (n + 1);
    }
    return false;
#endif
}

void invalidGLCall()
{
    // Use this to cause core dump so we have the stack.
//    printf("%s");
    throw Exception(AVG_ERR_VIDEO_GENERAL, "Illegal gl entry point called.");
    // Cause core dump.
}

GLfunction getFuzzyProcAddress(const char * psz)
{
    GLfunction pProc = (GLfunction)SDL_GL_GetProcAddress(psz);
    if (!pProc) {
        string s = string(psz)+"ARB";
        pProc = (GLfunction)SDL_GL_GetProcAddress(s.c_str());
    }
    if (!pProc) {
        pProc = invalidGLCall;
//        AVG_TRACE(Logger::WARNING, "Couldn't initialize pointer to " << psz);
    } else {
//        AVG_TRACE(Logger::WARNING, "Pointer to " << psz << " initialized.");
    }
    return pProc;
}
#ifdef linux
GLfunction getglXProcAddress(const char * psz)
{
    GLfunction pProc = (GLfunction)glXGetProcAddress((const GLubyte *)psz);
    //GLfunction pProc = (GLfunction)glXGetProcAddressARB((const GLubyte *)psz);
    if (!pProc) {
        pProc = invalidGLCall;
    }
    return pProc;
}
#endif
namespace glproc {

    void init() {
#ifdef __APPLE__
        OSStatus err = aglInitEntryPoints();
        if (noErr != err) {
            AVG_TRACE(Logger::ERROR, 
                    "Couldn't initialize Apple OpenGL entry points.");
        }
#endif    
        GenBuffers = (PFNGLGENBUFFERSPROC)getFuzzyProcAddress("glGenBuffers");
        BufferData = (PFNGLBUFFERDATAPROC)getFuzzyProcAddress("glBufferData");
        DeleteBuffers = (PFNGLDELETEBUFFERSPROC)getFuzzyProcAddress("glDeleteBuffers");
        BindBuffer = (PFNGLBINDBUFFERPROC)getFuzzyProcAddress("glBindBuffer");
        MapBuffer = (PFNGLMAPBUFFERPROC)getFuzzyProcAddress("glMapBuffer");
        UnmapBuffer = (PFNGLUNMAPBUFFERPROC)getFuzzyProcAddress("glUnmapBuffer");

        CreateShaderObject = (PFNGLCREATESHADEROBJECTARBPROC)
                getFuzzyProcAddress("glCreateShaderObject");
        ShaderSource = (PFNGLSHADERSOURCEARBPROC)
                getFuzzyProcAddress("glShaderSource");
        CompileShader = (PFNGLCOMPILESHADERARBPROC)
                getFuzzyProcAddress("glCompileShader");
        CreateProgramObject = (PFNGLCREATEPROGRAMOBJECTARBPROC)
                getFuzzyProcAddress("glCreateProgramObject");
        AttachObject = (PFNGLATTACHOBJECTARBPROC)
                getFuzzyProcAddress("glAttachObject");
        LinkProgram = (PFNGLLINKPROGRAMARBPROC)getFuzzyProcAddress("glLinkProgram");
        GetObjectParameteriv = (PFNGLGETOBJECTPARAMETERIVARBPROC)
                getFuzzyProcAddress("glGetObjectParameteriv");
        GetInfoLog = (PFNGLGETINFOLOGARBPROC)getFuzzyProcAddress("glGetInfoLog");
        UseProgramObject =(PFNGLUSEPROGRAMOBJECTARBPROC) 
                getFuzzyProcAddress("glUseProgramObject");
        GetUniformLocation = (PFNGLGETUNIFORMLOCATIONARBPROC)
                getFuzzyProcAddress("glGetUniformLocation");
        Uniform1i = (PFNGLUNIFORM1IARBPROC)getFuzzyProcAddress("glUniform1i");
        BlendEquation = (PFNGLBLENDEQUATIONPROC)getFuzzyProcAddress("glBlendEquation");
        ActiveTexture = (PFNGLACTIVETEXTUREPROC)getFuzzyProcAddress("glActiveTexture");
#ifdef linux
        SwapIntervalSGI = (PFNGLXSWAPINTERVALSGIPROC)getglXProcAddress("glXSwapIntervalSGI");
        WaitVideoSyncSGI = (PFNGLXWAITVIDEOSYNCSGIPROC)getglXProcAddress("glXWaitVideoSyncSGI");
#endif
    }
}

}
