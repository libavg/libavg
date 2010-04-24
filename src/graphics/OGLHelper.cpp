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

#include "../base/Logger.h"
#include "../base/Exception.h"

#ifndef _WIN32
#include <dlfcn.h>
#endif

#include <cstdio>
#include <iostream>
#include <sstream>
#include <cstring>
#include <cstdlib>

#if defined(__APPLE__)
#include <OpenGL/OpenGL.h>
#endif

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
    PFNGLUNIFORM2FARBPROC Uniform2f;
    PFNGLUNIFORM1FVARBPROC Uniform1fv;

    PFNGLBLENDFUNCSEPARATEPROC BlendFuncSeparate;
    PFNGLBLENDEQUATIONPROC BlendEquation;
    PFNGLACTIVETEXTUREPROC ActiveTexture;
    PFNGLGENERATEMIPMAPEXTPROC GenerateMipmap;

    PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC CheckFramebufferStatus;
    PFNGLGENFRAMEBUFFERSEXTPROC GenFramebuffers;
    PFNGLBINDFRAMEBUFFEREXTPROC BindFramebuffer;
    PFNGLFRAMEBUFFERTEXTURE2DEXTPROC FramebufferTexture2D;
    PFNGLDELETEFRAMEBUFFERSEXTPROC DeleteFramebuffers;
    PFNGLGENRENDERBUFFERSEXTPROC GenRenderbuffers;
    PFNGLBINDRENDERBUFFEREXTPROC BindRenderbuffer;
    PFNGLRENDERBUFFERSTORAGEEXTPROC RenderbufferStorage;
    PFNGLRENDERBUFFERSTORAGEMULTISAMPLEEXTPROC RenderbufferStorageMultisample;
    PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC FramebufferRenderbuffer;
    PFNGLBLITFRAMEBUFFEREXTPROC BlitFramebuffer;
    PFNGLDELETERENDERBUFFERSEXTPROC DeleteRenderbuffers;
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
        AVG_ASSERT(false);
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
        AVG_TRACE(Logger::ERROR, where+":"+szErr);
        AVG_ASSERT(false);
    }
}
#endif

bool queryOGLExtension(const char *extName)
{
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
            XCloseDisplay(display);
            return true;
        }
        p += (n + 1);
    }
    XCloseDisplay(display);
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

string AVG_API oglModeToString(int mode)
{
    switch (mode) {
        case GL_ALPHA:
            return "GL_ALPHA";
        case GL_RGB:
            return "GL_RGB";
        case GL_RGBA:
            return "GL_RGBA";
        case GL_BGR:
            return "GL_BGR";
        case GL_BGRA:
            return "GL_BGRA";
        default:
            return "UNKNOWN";
    }
}

#ifdef _WIN32
#define GL_ALL_CLIENT_ATTRIB_BITS GL_CLIENT_ALL_ATTRIB_BITS 
#endif

void pushGLState()
{
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glPushClientAttrib(GL_ALL_CLIENT_ATTRIB_BITS);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glMatrixMode(GL_TEXTURE);
    glPushMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "saveGLState()");
}

void popGLState()
{
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glMatrixMode(GL_TEXTURE);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glPopClientAttrib();
    glPopAttrib();
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "popGLState()");
}

#if defined(__APPLE__)
CGLContextObj g_AVGGLContext;
#elif defined(__linux__)
Display* g_pAVGGLDisplay;
GLXDrawable g_pAVGGLDrawable;
GLXContext g_AVGGLContext;
#elif defined(_WIN32)
HDC g_AVGGLHDC;
HGLRC g_AVGGLContext;
#endif

void AVG_API saveAVGGLContext()
{
#if defined(__APPLE__)
    g_AVGGLContext = CGLGetCurrentContext();
#elif defined(__linux__)
    g_pAVGGLDisplay = glXGetCurrentDisplay();
    g_pAVGGLDrawable = glXGetCurrentDrawable();
    g_AVGGLContext = glXGetCurrentContext();
#elif defined(_WIN32)
    g_AVGGLHDC = wglGetCurrentDC();
    g_AVGGLContext = wglGetCurrentContext();
#endif
}

void AVG_API restoreAVGGLContext()
{
    AVG_ASSERT(g_AVGGLContext);
#if defined(__APPLE__)
    CGLSetCurrentContext(g_AVGGLContext);
#elif defined(__linux__)
    glXMakeCurrent(g_pAVGGLDisplay, g_pAVGGLDrawable, g_AVGGLContext);
#elif defined(_WIN32)
    wglMakeCurrent(g_AVGGLHDC, g_AVGGLContext);
#endif
}

void invalidGLCall()
{
    // Use this to cause core dump so we have the stack.
//    printf("%s");
    throw Exception(AVG_ERR_VIDEO_GENERAL, "Illegal gl entry point called.");
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
    AVG_ASSERT(glproc::s_hGLLib);
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
        
        GenBuffers = (PFNGLGENBUFFERSPROC)getFuzzyProcAddress("glGenBuffers");
        BufferData = (PFNGLBUFFERDATAPROC)getFuzzyProcAddress("glBufferData");
        BufferSubData = (PFNGLBUFFERSUBDATAPROC)getFuzzyProcAddress("glBufferSubData");
        DeleteBuffers = (PFNGLDELETEBUFFERSPROC)getFuzzyProcAddress("glDeleteBuffers");
        BindBuffer = (PFNGLBINDBUFFERPROC)getFuzzyProcAddress("glBindBuffer");
        MapBuffer = (PFNGLMAPBUFFERPROC)getFuzzyProcAddress("glMapBuffer");
        UnmapBuffer = (PFNGLUNMAPBUFFERPROC)getFuzzyProcAddress("glUnmapBuffer");
        GetBufferSubData = (PFNGLGETBUFFERSUBDATAPROC)getFuzzyProcAddress
                ("glGetBufferSubData");

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
        Uniform2f = (PFNGLUNIFORM2FARBPROC)getFuzzyProcAddress("glUniform2f");
        Uniform1fv = (PFNGLUNIFORM1FVARBPROC)getFuzzyProcAddress("glUniform1fv");
        
        BlendFuncSeparate = (PFNGLBLENDFUNCSEPARATEPROC)
                getFuzzyProcAddress("glBlendFuncSeparate");
        BlendEquation = (PFNGLBLENDEQUATIONPROC)getFuzzyProcAddress("glBlendEquation");
        ActiveTexture = (PFNGLACTIVETEXTUREPROC)getFuzzyProcAddress("glActiveTexture");
        GenerateMipmap = (PFNGLGENERATEMIPMAPEXTPROC)getFuzzyProcAddress
                ("glGenerateMipmap");
        
        CheckFramebufferStatus = (PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC)
                getFuzzyProcAddress("glCheckFramebufferStatus");
        GenFramebuffers = (PFNGLGENFRAMEBUFFERSEXTPROC)
                getFuzzyProcAddress("glGenFramebuffers");
        BindFramebuffer = (PFNGLBINDFRAMEBUFFEREXTPROC)
                getFuzzyProcAddress("glBindFramebuffer");
        FramebufferTexture2D = (PFNGLFRAMEBUFFERTEXTURE2DEXTPROC)
                getFuzzyProcAddress("glFramebufferTexture2D");
        DeleteFramebuffers = (PFNGLDELETEFRAMEBUFFERSEXTPROC)
                getFuzzyProcAddress("glDeleteFramebuffers");
        GenRenderbuffers = (PFNGLGENRENDERBUFFERSEXTPROC)
                getFuzzyProcAddress("glGenRenderbuffers");
        BindRenderbuffer = (PFNGLBINDRENDERBUFFEREXTPROC)
                getFuzzyProcAddress("glBindRenderbuffer");
        RenderbufferStorage= (PFNGLRENDERBUFFERSTORAGEEXTPROC)
                getFuzzyProcAddress("glRenderbufferStorage");
        RenderbufferStorageMultisample = (PFNGLRENDERBUFFERSTORAGEMULTISAMPLEEXTPROC)
                getFuzzyProcAddress("glRenderbufferStorageMultisample");
        FramebufferRenderbuffer = (PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC)
                getFuzzyProcAddress("glFramebufferRenderbuffer");
        BlitFramebuffer = (PFNGLBLITFRAMEBUFFEREXTPROC)
                getFuzzyProcAddress("glBlitFramebuffer");
        DeleteRenderbuffers = (PFNGLDELETERENDERBUFFERSEXTPROC)
                getFuzzyProcAddress("glDeleteRenderbuffers");
#ifdef linux
        SwapIntervalSGI = (PFNGLXSWAPINTERVALSGIPROC)
                getglXProcAddress("glXSwapIntervalSGI");
        WaitVideoSyncSGI = (PFNGLXWAITVIDEOSYNCSGIPROC)
                getglXProcAddress("glXWaitVideoSyncSGI");
#endif

#ifdef _WIN32
        SwapIntervalEXT = (PFNWGLEXTSWAPCONTROLPROC) 
                getwglProcAddress("wglSwapIntervalEXT");
#endif
    }
}

}
