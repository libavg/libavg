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

#include "OGLHelper.h"

#include "../base/Logger.h"
#include "../base/Exception.h"

#include "GLContext.h"

#ifndef _WIN32
#include <dlfcn.h>
#endif

#include <cstdio>
#include <iostream>
#include <sstream>
#include <cstring>
#include <cstdlib>
#include <assert.h>

#if defined(__APPLE__)
#include <OpenGL/OpenGL.h>
#endif

using namespace std;

namespace avg {

namespace glproc {
#ifndef AVG_ENABLE_EGL
    PFNGLBUFFERSUBDATAPROC BufferSubData;
    PFNGLGETBUFFERSUBDATAPROC GetBufferSubData;
    PFNGLBLITFRAMEBUFFERPROC BlitFramebuffer;
    PFNGLDRAWBUFFERSPROC DrawBuffers;
    PFNGLDRAWRANGEELEMENTSPROC DrawRangeElements;
    PFNGLGETOBJECTPARAMETERIVARBPROC GetObjectParameteriv;
#endif
    PFNGLGENBUFFERSPROC GenBuffers;
    PFNGLBUFFERDATAPROC BufferData;
    PFNGLDEBUGMESSAGECALLBACKPROC DebugMessageCallback;
    PFNGLDELETEBUFFERSPROC DeleteBuffers;
    PFNGLBINDBUFFERPROC BindBuffer;
    PFNGLMAPBUFFERPROC MapBuffer;
    PFNGLUNMAPBUFFERPROC UnmapBuffer;

    PFNGLCREATESHADERPROC CreateShader;
    PFNGLSHADERSOURCEPROC ShaderSource;
    PFNGLCOMPILESHADERPROC CompileShader;
    PFNGLCREATEPROGRAMPROC CreateProgram;
    PFNGLATTACHSHADERPROC AttachShader;
    PFNGLLINKPROGRAMPROC LinkProgram;
    PFNGLGETSHADERIVPROC GetShaderiv;
    PFNGLGETPROGRAMIVPROC GetProgramiv;
    PFNGLGETSHADERINFOLOGPROC GetShaderInfoLog;
    PFNGLGETPROGRAMINFOLOGPROC GetProgramInfoLog;
    PFNGLUSEPROGRAMPROC UseProgram;
    PFNGLGETUNIFORMLOCATIONPROC GetUniformLocation;
    PFNGLUNIFORM1IPROC Uniform1i;
    PFNGLUNIFORM1FPROC Uniform1f;
    PFNGLUNIFORM2FPROC Uniform2f;
    PFNGLUNIFORM3FPROC Uniform3f;
    PFNGLUNIFORM4FPROC Uniform4f;
    PFNGLUNIFORM1FVPROC Uniform1fv;
    PFNGLUNIFORMMATRIX4FVPROC UniformMatrix4fv;

    PFNGLBLENDFUNCSEPARATEPROC BlendFuncSeparate;
    PFNGLBLENDEQUATIONPROC BlendEquation;
    PFNGLBLENDCOLORPROC BlendColor;
    PFNGLACTIVETEXTUREPROC ActiveTexture;
    PFNGLGENERATEMIPMAPPROC GenerateMipmap;

    PFNGLCHECKFRAMEBUFFERSTATUSPROC CheckFramebufferStatus;
    PFNGLGENFRAMEBUFFERSPROC GenFramebuffers;
    PFNGLBINDFRAMEBUFFERPROC BindFramebuffer;
    PFNGLFRAMEBUFFERTEXTURE2DPROC FramebufferTexture2D;
    PFNGLDELETEFRAMEBUFFERSPROC DeleteFramebuffers;
    PFNGLGENRENDERBUFFERSPROC GenRenderbuffers;
    PFNGLBINDRENDERBUFFERPROC BindRenderbuffer;
    PFNGLRENDERBUFFERSTORAGEPROC RenderbufferStorage;
    PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC RenderbufferStorageMultisample;
    PFNGLFRAMEBUFFERRENDERBUFFERPROC FramebufferRenderbuffer;
    PFNGLDELETERENDERBUFFERSPROC DeleteRenderbuffers;
    PFNGLVERTEXATTRIBPOINTERPROC VertexAttribPointer;
    PFNGLENABLEVERTEXATTRIBARRAYPROC EnableVertexAttribArray;
    PFNGLBINDATTRIBLOCATIONPROC BindAttribLocation;
#if defined(linux) && !defined(AVG_ENABLE_EGL)
    PFNGLXSWAPINTERVALEXTPROC SwapIntervalEXT;
#endif
#ifdef _WIN32
    PFNWGLSWAPINTERVALEXTPROC SwapIntervalEXT;
#endif

    void * s_hGLLib = 0;
}

bool queryOGLExtension(const char *extName)
{
    char *p;
    size_t extNameLen = strlen(extName);

    p = (char *)glGetString(GL_EXTENSIONS);
    AVG_ASSERT(p != 0);
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

bool queryGLXExtension(const char *extName)
{
#if (defined __APPLE__) || (defined _WIN32) || (defined AVG_ENABLE_EGL)
    return false;
#else
    int extNameLen = strlen(extName);

    Display * display = XOpenDisplay(0);
    char * p = (char *)glXQueryExtensionsString(display, DefaultScreen(display));
    if (NULL == p) {
        throw Exception(AVG_ERR_VIDEO_GENERAL, "Couldn't get GLX extension string.");
    }

    char * end = p + strlen(p);

    while (p < end) {
        int n = strcspn(p, " ");
        if ((extNameLen == n) && (strncmp(extName, p, n) == 0)) {
// If we close the display connection on some drivers (at least fglrx/Ubuntu 11.04),
// libavg crashes soon afterwards.
//            XCloseDisplay(display);
            return true;
        }
        p += (n + 1);
    }
//    XCloseDisplay(display);
    return false;
#endif
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
#ifdef AVG_ENABLE_EGL
        case GL_BGRA_EXT:
            return "GL_BGRA_EXT";
#else
        case GL_BGR:
            return "GL_BGR";
        case GL_BGRA:
            return "GL_BGRA";
#endif
        default:
            return "UNKNOWN";
    }
}

#ifdef _WIN32
#define GL_ALL_CLIENT_ATTRIB_BITS GL_CLIENT_ALL_ATTRIB_BITS 
#endif

string oglMemoryMode2String(OGLMemoryMode mode)
{
    switch (mode) {
        case MM_PBO:
            return "PBO";
        case MM_OGL:
            return "OGL";
        default:
            return "invalid gl mem mode";
    }
}

void AVG_API clearGLBuffers(GLbitfield mask, bool bOpaque)
{
    float alpha;
    if (bOpaque) {
        alpha = 1.0;
    } else {
        alpha = 0.0;
    }
    glClearColor(0.0, 0.0, 0.0, alpha); 
    if (mask & GL_STENCIL_BUFFER_BIT) {
        glStencilMask(~0);
        glClearStencil(0);
    }
    glClear(mask);
    GLContext::checkError("clearGLBuffers()");
    if (mask & GL_STENCIL_BUFFER_BIT) {
        glStencilMask(0);
    }
}

void invalidGLCall()
{
    assert(false);
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
#ifdef AVG_ENABLE_EGL
    const char * pszFName = "libGLESv2.so";
#else
    const char * pszFName = "libGL.so.1";
#endif
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
        throw Exception(AVG_ERR_VIDEO_GENERAL, 
                string("wglGetProcAddress("+sName+") failed: ") + szErr);
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
        string s = string(psz)+"OES";
        pProc = getProcAddress(s);
    }
    if (!pProc) {
        pProc = invalidGLCall;
    }
    return pProc;
}

#if defined(linux) && !defined(AVG_ENABLE_EGL)
GLfunction getglXProcAddress(const char * psz)
{
    GLfunction pProc = (GLfunction)glXGetProcAddress((const GLubyte *)psz);
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
        DeleteBuffers = (PFNGLDELETEBUFFERSPROC)getFuzzyProcAddress("glDeleteBuffers");
        BindBuffer = (PFNGLBINDBUFFERPROC)getFuzzyProcAddress("glBindBuffer");
        MapBuffer = (PFNGLMAPBUFFERPROC)getFuzzyProcAddress("glMapBuffer");
        UnmapBuffer = (PFNGLUNMAPBUFFERPROC)getFuzzyProcAddress("glUnmapBuffer");

        CreateShader = (PFNGLCREATESHADERPROC)getFuzzyProcAddress("glCreateShader");
        ShaderSource = (PFNGLSHADERSOURCEPROC)getFuzzyProcAddress("glShaderSource");
        CompileShader = (PFNGLCOMPILESHADERPROC)getFuzzyProcAddress("glCompileShader");
        CreateProgram= (PFNGLCREATEPROGRAMPROC)getFuzzyProcAddress("glCreateProgram");
        AttachShader = (PFNGLATTACHSHADERPROC)getFuzzyProcAddress("glAttachShader");
        LinkProgram = (PFNGLLINKPROGRAMPROC)getFuzzyProcAddress("glLinkProgram");
        GetShaderiv = (PFNGLGETSHADERIVPROC)getFuzzyProcAddress("glGetShaderiv");
        GetProgramiv = (PFNGLGETPROGRAMIVPROC)getFuzzyProcAddress("glGetProgramiv");
        GetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC)
                getFuzzyProcAddress("glGetShaderInfoLog");
        GetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC)
                getFuzzyProcAddress("glGetProgramInfoLog");
        UseProgram =(PFNGLUSEPROGRAMPROC) getFuzzyProcAddress("glUseProgram");
        GetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC)
                getFuzzyProcAddress("glGetUniformLocation");
        Uniform1i = (PFNGLUNIFORM1IPROC)getFuzzyProcAddress("glUniform1i");
        Uniform1f = (PFNGLUNIFORM1FPROC)getFuzzyProcAddress("glUniform1f");
        Uniform2f = (PFNGLUNIFORM2FPROC)getFuzzyProcAddress("glUniform2f");
        Uniform3f = (PFNGLUNIFORM3FPROC)getFuzzyProcAddress("glUniform3f");
        Uniform4f = (PFNGLUNIFORM4FPROC)getFuzzyProcAddress("glUniform4f");
        Uniform1fv = (PFNGLUNIFORM1FVPROC)getFuzzyProcAddress("glUniform1fv");
        UniformMatrix4fv = (PFNGLUNIFORMMATRIX4FVPROC)
                getFuzzyProcAddress("glUniformMatrix4fv");
        
        BlendFuncSeparate = (PFNGLBLENDFUNCSEPARATEPROC)
                getFuzzyProcAddress("glBlendFuncSeparate");
        BlendEquation = (PFNGLBLENDEQUATIONPROC)getFuzzyProcAddress("glBlendEquation");
        BlendColor = (PFNGLBLENDCOLORPROC)getFuzzyProcAddress("glBlendColor");
        ActiveTexture = (PFNGLACTIVETEXTUREPROC)getFuzzyProcAddress("glActiveTexture");
        GenerateMipmap = (PFNGLGENERATEMIPMAPPROC)getFuzzyProcAddress
                ("glGenerateMipmap");
        
        CheckFramebufferStatus = (PFNGLCHECKFRAMEBUFFERSTATUSPROC)
                getFuzzyProcAddress("glCheckFramebufferStatus");
        GenFramebuffers = (PFNGLGENFRAMEBUFFERSPROC)
                getFuzzyProcAddress("glGenFramebuffers");
        BindFramebuffer = (PFNGLBINDFRAMEBUFFERPROC)
                getFuzzyProcAddress("glBindFramebuffer");
        FramebufferTexture2D = (PFNGLFRAMEBUFFERTEXTURE2DPROC)
                getFuzzyProcAddress("glFramebufferTexture2D");
        DeleteFramebuffers = (PFNGLDELETEFRAMEBUFFERSPROC)
                getFuzzyProcAddress("glDeleteFramebuffers");
        GenRenderbuffers = (PFNGLGENRENDERBUFFERSPROC)
                getFuzzyProcAddress("glGenRenderbuffers");
        BindRenderbuffer = (PFNGLBINDRENDERBUFFERPROC)
                getFuzzyProcAddress("glBindRenderbuffer");
        RenderbufferStorage= (PFNGLRENDERBUFFERSTORAGEPROC)
                getFuzzyProcAddress("glRenderbufferStorage");
        RenderbufferStorageMultisample = (PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC)
                getFuzzyProcAddress("glRenderbufferStorageMultisample");
        FramebufferRenderbuffer = (PFNGLFRAMEBUFFERRENDERBUFFERPROC)
                getFuzzyProcAddress("glFramebufferRenderbuffer");

        DeleteRenderbuffers = (PFNGLDELETERENDERBUFFERSPROC)
                getFuzzyProcAddress("glDeleteRenderbuffers");
#ifndef AVG_ENABLE_EGL
        BufferSubData = (PFNGLBUFFERSUBDATAPROC)getFuzzyProcAddress("glBufferSubData");
        GetBufferSubData = (PFNGLGETBUFFERSUBDATAPROC)getFuzzyProcAddress
            ("glGetBufferSubData");
        GetObjectParameteriv = (PFNGLGETOBJECTPARAMETERIVARBPROC)
            getFuzzyProcAddress("glGetObjectParameteriv");

        BlitFramebuffer = (PFNGLBLITFRAMEBUFFERPROC)
                getFuzzyProcAddress("glBlitFramebuffer");
        DrawBuffers = (PFNGLDRAWBUFFERSPROC)getFuzzyProcAddress("glDrawBuffers");
        DrawRangeElements = (PFNGLDRAWRANGEELEMENTSPROC)
                getFuzzyProcAddress("glDrawRangeElements");
        DebugMessageCallback = (PFNGLDEBUGMESSAGECALLBACKARBPROC)
                getFuzzyProcAddress("glDebugMessageCallback");
#endif
        VertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERPROC)
                getFuzzyProcAddress("glVertexAttribPointer");
        EnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC)
                getFuzzyProcAddress("glEnableVertexAttribArray");
        BindAttribLocation = (PFNGLBINDATTRIBLOCATIONPROC)
                getFuzzyProcAddress("glBindAttribLocation");
#if defined(linux) && !defined(AVG_ENABLE_EGL)
        SwapIntervalEXT = (PFNGLXSWAPINTERVALEXTPROC)
                getglXProcAddress("glXSwapIntervalEXT");
#endif
#ifdef _WIN32
        SwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)
                getwglProcAddress("wglSwapIntervalEXT");
#endif
    }
}

}
