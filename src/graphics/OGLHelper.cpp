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

#include "OGLHelper.h"

#ifdef __APPLE__
#include "AppleGLHelper.h"
#endif

#include "../base/Logger.h"
#include "../base/Exception.h"

#ifndef _WIN32
#include <dlfcn.h>
#endif

#include <assert.h>
#include <cstdio>
#include <iostream>
#include <sstream>
#include <cstring>

using namespace std;

namespace avg {

namespace glproc {
    PFNGLGENBUFFERSPROC GenBuffers;
    PFNGLBUFFERDATAPROC BufferData;
    PFNGLBUFFERSUBDATAPROC BufferSubData;
    PFNGLDELETEBUFFERSPROC DeleteBuffers;
    PFNGLBINDBUFFERPROC BindBuffer;
    PFNGLMAPBUFFERPROC MapBuffer;
    PFNGLUNMAPBUFFERPROC UnmapBuffer;
    PFNGLGETBUFFERSUBDATAPROC GetBufferSubData;

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
    PFNGLUNIFORM1FARBPROC Uniform1f;
    PFNGLUNIFORM1FVARBPROC Uniform1fv;
    PFNGLBLENDEQUATIONPROC BlendEquation;
    PFNGLACTIVETEXTUREPROC ActiveTexture;
    PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC CheckFramebufferStatus;
    PFNGLGENFRAMEBUFFERSEXTPROC GenFramebuffers;
    PFNGLBINDFRAMEBUFFEREXTPROC BindFramebuffer;
    PFNGLFRAMEBUFFERTEXTURE2DEXTPROC FramebufferTexture2D;
    PFNGLDELETEFRAMEBUFFERSEXTPROC DeleteFramebuffers;
#ifdef linux
    PFNGLXSWAPINTERVALSGIPROC SwapIntervalSGI;
    PFNGLXWAITVIDEOSYNCSGIPROC WaitVideoSyncSGI;
#endif
#ifdef _WIN32
    PFNWGLEXTSWAPCONTROLPROC SwapIntervalEXT;
#endif

    void * s_hGLLib = 0;
}

void OGLErrorCheck(int avgcode, const char * where) 
{
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

#ifdef _WIN32
void winOGLErrorCheck(BOOL bOK, const string & where) 
{
    if (!bOK) {
        char szErr[512];
        FormatMessage((FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM),
                0, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                szErr, 512, 0);
        throw Exception(AVG_ERR_VIDEO_GENERAL, where+":"+
                + szErr);
    }
}
#endif

bool queryOGLExtension(const char *extName)
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

bool queryGLXExtension(const char *extName) {
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

void getGLVersion(int & major, int& minor)
{
    const char* pVersion = (const char*)glGetString(GL_VERSION);
    sscanf(pVersion, "%d.%d", &major, &minor);
}

void getGLShadingLanguageVersion(int & major, int& minor)
{
    int glMajor, glMinor;
    getGLVersion(glMajor, glMinor);

    major = minor = 0;
    if (glMajor == 1) {
        if (queryOGLExtension("GL_ARB_shading_language_100")) {
            major = 1;
            minor = 0;
        }
    } else {
        const char* pVersion = (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION);
        sscanf(pVersion, "%d.%d", &major, &minor);
    }
}

void invalidGLCall()
{
    // Use this to cause core dump so we have the stack.
//    printf("%s");
    throw Exception(AVG_ERR_VIDEO_GENERAL, "Illegal gl entry point called.");
    // Cause core dump.
}

void loadGLLibrary()
{
#ifdef _WIN32
    const char * pszFName = "OPENGL32.DLL";
    char szErr[512];

    glproc::s_hGLLib = (void *)LoadLibrary(pszFName);

    if (glproc::s_hGLLib == 0) {
        FormatMessage((FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM),
                0, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                szErr, 512, 0);
        throw Exception(AVG_ERR_VIDEO_GENERAL, string("Loading ") + pszFName + "failed: " 
                + szErr);
    }
#else
#ifdef __APPLE__
    const char * pszFName = "/System/Library/Frameworks/OpenGL.framework/OpenGL";
#else
    const char * pszFName = "libGL.so.1";
#endif
    glproc::s_hGLLib = dlopen(pszFName, RTLD_NOW);
    if (glproc::s_hGLLib == 0) {
        const char * pszErr = (char *)dlerror();
        throw Exception(AVG_ERR_VIDEO_GENERAL, string("Loading ") + pszFName + "failed: " 
                + pszErr);
    }
#endif
}

GLfunction getProcAddress(const string& sName)
{
    assert(glproc::s_hGLLib);
#ifdef _WIN32
    GLfunction pProc = (GLfunction)wglGetProcAddress(sName.c_str());
/*
    if (!pProc) {
        char szErr[512];
        FormatMessage((FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM),
                0, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                szErr, 512, 0);
        throw Exception(AVG_ERR_VIDEO_GENERAL, string("wglGetProcAddress("+sName+") failed: ") + szErr);
    }
*/
#else
    GLfunction pProc = (GLfunction)dlsym(glproc::s_hGLLib, sName.c_str());
    if (!pProc) {
        // If the name didn't work, try using an underscore :-).
        string sName_ = string("_")+sName;
        pProc = (GLfunction)dlsym(glproc::s_hGLLib, sName_.c_str());
    }
#endif
    return(pProc);
}

GLfunction getFuzzyProcAddress(const char * psz)
{
    GLfunction pProc = getProcAddress(psz);
    if (!pProc) {
        string s = string(psz)+"EXT";
        pProc = getProcAddress(s);
    }
    if (!pProc) {
        string s = string(psz)+"ARB";
        pProc = getProcAddress(s);
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

#ifdef _WIN32
GLfunction getwglProcAddress(const char * psz)
{
    GLfunction pProc = (GLfunction)wglGetProcAddress((LPCSTR)psz);
    if (!pProc) {
        pProc = invalidGLCall;
    }
    return pProc;
}
#endif

namespace glproc {

    void init() {
        static bool s_bInitialized = false;
        if (s_bInitialized) {
            return;
        }
        s_bInitialized = true;
        loadGLLibrary();
#ifdef __APPLE__
        OSStatus err = aglInitEntryPoints();
        if (noErr != err) {
            AVG_TRACE(Logger::ERROR, 
                    "Couldn't initialize Apple OpenGL entry points.");
        }
#endif    
        GenBuffers = (PFNGLGENBUFFERSPROC)getFuzzyProcAddress("glGenBuffers");
        BufferData = (PFNGLBUFFERDATAPROC)getFuzzyProcAddress("glBufferData");
        BufferSubData = (PFNGLBUFFERSUBDATAPROC)getFuzzyProcAddress("glBufferSubData");
        DeleteBuffers = (PFNGLDELETEBUFFERSPROC)getFuzzyProcAddress("glDeleteBuffers");
        BindBuffer = (PFNGLBINDBUFFERPROC)getFuzzyProcAddress("glBindBuffer");
        MapBuffer = (PFNGLMAPBUFFERPROC)getFuzzyProcAddress("glMapBuffer");
        UnmapBuffer = (PFNGLUNMAPBUFFERPROC)getFuzzyProcAddress("glUnmapBuffer");
        GetBufferSubData = (PFNGLGETBUFFERSUBDATAPROC)getFuzzyProcAddress("glGetBufferSubData");

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
        Uniform1f = (PFNGLUNIFORM1FARBPROC)getFuzzyProcAddress("glUniform1f");
        Uniform1fv = (PFNGLUNIFORM1FVARBPROC)getFuzzyProcAddress("glUniform1fv");
        BlendEquation = (PFNGLBLENDEQUATIONPROC)getFuzzyProcAddress("glBlendEquation");
        ActiveTexture = (PFNGLACTIVETEXTUREPROC)getFuzzyProcAddress("glActiveTexture");
        CheckFramebufferStatus = 
                (PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC)getFuzzyProcAddress("glCheckFramebufferStatus");
        GenFramebuffers = (PFNGLGENFRAMEBUFFERSEXTPROC)getFuzzyProcAddress("glGenFramebuffers");
        BindFramebuffer = (PFNGLBINDFRAMEBUFFEREXTPROC)getFuzzyProcAddress("glBindFramebuffer");
        FramebufferTexture2D = 
                (PFNGLFRAMEBUFFERTEXTURE2DEXTPROC)getFuzzyProcAddress("glFramebufferTexture2D");
        DeleteFramebuffers = 
                (PFNGLDELETEFRAMEBUFFERSEXTPROC)getFuzzyProcAddress("glDeleteFramebuffers");
#ifdef linux
        SwapIntervalSGI = (PFNGLXSWAPINTERVALSGIPROC)getglXProcAddress("glXSwapIntervalSGI");
        WaitVideoSyncSGI = (PFNGLXWAITVIDEOSYNCSGIPROC)getglXProcAddress("glXWaitVideoSyncSGI");
#endif

#ifdef _WIN32
        SwapIntervalEXT = (PFNWGLEXTSWAPCONTROLPROC) getwglProcAddress("wglSwapIntervalEXT");
#endif
    }
}

}
