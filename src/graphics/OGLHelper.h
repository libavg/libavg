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

#ifndef _OGLHelper_H_
#define _OGLHelper_H_

#ifdef _WIN32
#include "../api.h"
#include <windows.h>
#undef ERROR
#undef WARNING
#include <GL/gl.h>
#include <GL/glu.h>
#include "GL/glext.h"
#else
#include "GL/gl.h"
#include "GL/glu.h"
#endif
#ifdef linux
#define GLX_GLXEXT_PROTOTYPES
#include <GL/glx.h>
#endif
#ifdef _WIN32
//TODO: Does anyone know where these are declared?
typedef void (APIENTRY *PFNWGLEXTSWAPCONTROLPROC) (int);
typedef int (*PFNWGLEXTGETSWAPINTERVALPROC) (void);
#endif

#include <string>

namespace avg {

void OGLErrorCheck(int avgcode, const char * where);
#ifdef _WIN32
void winOGLErrorCheck(BOOL bOK, const std::string & where);
#endif
bool queryOGLExtension(const char *extName);
bool queryGLXExtension(const char *extName);
void getGLVersion(int & major, int& minor);
void getGLShadingLanguageVersion(int & major, int& minor);

enum OGLMemoryMode { 
    OGL,  // Standard OpenGL
    PBO   // pixel buffer objects
};

typedef void (*GLfunction)();
GLfunction getFuzzyProcAddress(const char * psz);

namespace glproc {
    extern AVG_API PFNGLGENBUFFERSPROC GenBuffers;
    extern AVG_API PFNGLBUFFERDATAPROC BufferData;
    extern AVG_API PFNGLBUFFERSUBDATAPROC BufferSubData;
    extern AVG_API PFNGLDELETEBUFFERSPROC DeleteBuffers;
    extern AVG_API PFNGLBINDBUFFERPROC BindBuffer;
    extern AVG_API PFNGLMAPBUFFERPROC MapBuffer;
    extern AVG_API PFNGLUNMAPBUFFERPROC UnmapBuffer;
    extern AVG_API PFNGLGETBUFFERSUBDATAPROC GetBufferSubData;

    extern AVG_API PFNGLCREATESHADEROBJECTARBPROC CreateShaderObject;
    extern AVG_API PFNGLSHADERSOURCEARBPROC ShaderSource;
    extern AVG_API PFNGLCOMPILESHADERARBPROC CompileShader;
    extern AVG_API PFNGLCREATEPROGRAMOBJECTARBPROC CreateProgramObject;
    extern AVG_API PFNGLATTACHOBJECTARBPROC AttachObject;
    extern AVG_API PFNGLLINKPROGRAMARBPROC LinkProgram;
    extern AVG_API PFNGLGETOBJECTPARAMETERIVARBPROC GetObjectParameteriv;
    extern AVG_API PFNGLGETINFOLOGARBPROC GetInfoLog;
    extern AVG_API PFNGLUSEPROGRAMOBJECTARBPROC UseProgramObject;
    extern AVG_API PFNGLGETUNIFORMLOCATIONARBPROC GetUniformLocation;
    extern AVG_API PFNGLUNIFORM1IARBPROC Uniform1i;
    extern AVG_API PFNGLUNIFORM1FARBPROC Uniform1f;
    extern AVG_API PFNGLUNIFORM1FVARBPROC Uniform1fv;
    extern AVG_API PFNGLBLENDEQUATIONPROC BlendEquation;
    extern AVG_API PFNGLACTIVETEXTUREPROC ActiveTexture;

    extern AVG_API PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC CheckFramebufferStatus;
    extern AVG_API PFNGLGENFRAMEBUFFERSEXTPROC GenFramebuffers;
    extern AVG_API PFNGLBINDFRAMEBUFFEREXTPROC BindFramebuffer;
    extern AVG_API PFNGLFRAMEBUFFERTEXTURE2DEXTPROC FramebufferTexture2D;
    extern AVG_API PFNGLDELETEFRAMEBUFFERSEXTPROC DeleteFramebuffers;

#ifdef linux
    extern PFNGLXSWAPINTERVALSGIPROC SwapIntervalSGI;
    extern PFNGLXWAITVIDEOSYNCSGIPROC WaitVideoSyncSGI;
#endif
#ifdef _WIN32
    extern AVG_API PFNWGLEXTSWAPCONTROLPROC SwapIntervalEXT;
#endif
    void init();

    extern void * s_hGLLib;
}


}

#ifndef GL_TEXTURE_RECTANGLE_NV
#define GL_TEXTURE_RECTANGLE_NV           0x84F5
#endif

#endif
 
