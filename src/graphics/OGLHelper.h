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

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
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
#include "GL/glx.h"
#endif
#ifdef _WIN32
//TODO: Does anyone know where these are declared?
typedef void (APIENTRY *PFNWGLEXTSWAPCONTROLPROC) (int);
typedef int (*PFNWGLEXTGETSWAPINTERVALPROC) (void);
#endif

#include <string>

namespace avg {

void OGLErrorCheck(int avgcode, const std::string & where);
bool queryOGLExtension(char *extName);
bool queryGLXExtension(char *extName);

enum OGLMemoryMode { 
    OGL,  // Standard OpenGL
    PBO   // pixel buffer objects
};

typedef void (*GLfunction)();
GLfunction getFuzzyProcAddress(const char * psz);

namespace glproc {
    extern PFNGLGENBUFFERSPROC GenBuffers;
    extern PFNGLBUFFERDATAPROC BufferData;
    extern PFNGLDELETEBUFFERSPROC DeleteBuffers;
    extern PFNGLBINDBUFFERPROC BindBuffer;
    extern PFNGLMAPBUFFERPROC MapBuffer;
    extern PFNGLUNMAPBUFFERPROC UnmapBuffer;

    extern PFNGLCREATESHADEROBJECTARBPROC CreateShaderObject;
    extern PFNGLSHADERSOURCEARBPROC ShaderSource;
    extern PFNGLCOMPILESHADERARBPROC CompileShader;
    extern PFNGLCREATEPROGRAMOBJECTARBPROC CreateProgramObject;
    extern PFNGLATTACHOBJECTARBPROC AttachObject;
    extern PFNGLLINKPROGRAMARBPROC LinkProgram;
    extern PFNGLGETOBJECTPARAMETERIVARBPROC GetObjectParameteriv;
    extern PFNGLGETINFOLOGARBPROC GetInfoLog;
    extern PFNGLUSEPROGRAMOBJECTARBPROC UseProgramObject;
    extern PFNGLGETUNIFORMLOCATIONARBPROC GetUniformLocation;
    extern PFNGLUNIFORM1IARBPROC Uniform1i;
    extern PFNGLUNIFORM1FARBPROC Uniform1f;
    extern PFNGLBLENDEQUATIONPROC BlendEquation;
    extern PFNGLACTIVETEXTUREPROC ActiveTexture;

    extern PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC CheckFramebufferStatus;
    extern PFNGLGENFRAMEBUFFERSEXTPROC GenFramebuffers;
    extern PFNGLBINDFRAMEBUFFEREXTPROC BindFramebuffer;
    extern PFNGLFRAMEBUFFERTEXTURE2DEXTPROC FramebufferTexture2D;
    extern PFNGLDELETEFRAMEBUFFERSEXTPROC DeleteFramebuffers;

#ifdef linux
    extern PFNGLXSWAPINTERVALSGIPROC SwapIntervalSGI;
    extern PFNGLXWAITVIDEOSYNCSGIPROC WaitVideoSyncSGI;
#endif
#ifdef _WIN32
    extern PFNWGLEXTSWAPCONTROLPROC SwapIntervalEXT;
#endif
    void init();
}


}

// This should be in a system-wide gl header, but for some reason it isn't
// always there...
#ifndef GL_TEXTURE_RECTANGLE_NV
#define GL_TEXTURE_RECTANGLE_NV           0x84F5
#endif

#endif
 
