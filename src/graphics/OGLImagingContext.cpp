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

#include "OGLImagingContext.h"

#include "../base/Exception.h"

#ifdef _WIN32
// XXX: If the following includes are not there, the MS linker optimizes away
// the classes so they can't be used by plugins anymore (!). Adding /OPT:NOREF
// to the linker flags doesn't help. 
#include "IteratingGPUFilter.h"
#include "FilterGetAlpha.h"
#include "FilterErosion.h"
#include "FilterDilation.h"
#include "FilterGetAlpha.h"
#include "FilterResizeBilinear.h"
#include "FilterResizeGaussian.h"
#endif

#include <iostream>

namespace avg {

using namespace std;

#ifdef _WIN32
LONG WINAPI imagingWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{ 
    return DefWindowProc(hwnd, msg, wParam, lParam); 
} 

void registerWindowClass()
{
  static char * pClassName;
  if (pClassName) {
      return;
  }
  pClassName = "GLUT";

  HINSTANCE hInstance = GetModuleHandle(NULL);
  WNDCLASS  wc;
  memset(&wc, 0, sizeof(WNDCLASS));
  wc.style = CS_OWNDC;
  wc.lpfnWndProc = (WNDPROC)imagingWindowProc;
  wc.hInstance = hInstance;
  wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);
  wc.hCursor = LoadCursor(hInstance, IDC_ARROW);
  wc.hbrBackground = NULL;
  wc.lpszMenuName = NULL;
  wc.lpszClassName = pClassName;
  
  BOOL bOK = RegisterClass(&wc);
  AVG_ASSERT(bOK);
}
#endif

#ifdef linux
static bool s_bX11Error;
static int (*s_DefaultErrorHandler) (Display *, XErrorEvent *);

int X11ErrorHandler(Display * pDisplay, XErrorEvent * pErrEvent)
{
    cerr << "X11 error creating offscreen context: " << (int)(pErrEvent->request_code)
            << ", " << (int)(pErrEvent->minor_code) << endl;
    s_bX11Error = true;
    return 0;
}
#endif

OGLImagingContext::OGLImagingContext(const IntPoint & size)
{
#ifdef __APPLE__
    GLint attributes[] = {AGL_RGBA, AGL_ALL_RENDERERS,AGL_NONE};
    AGLPixelFormat format;
    format = aglChoosePixelFormat(NULL, 0, attributes);
    AVG_ASSERT(format);

    m_Context = aglCreateContext(format, NULL);
    AVG_ASSERT(m_Context);
    aglDestroyPixelFormat(format);

    bool bOk = aglSetCurrentContext(m_Context);
    AVG_ASSERT (bOk);
#else
#ifdef linux
    Display *dpy;
    dpy = XOpenDisplay(0);
    if (!dpy) {
        throw Exception(AVG_ERR_VIDEO_GENERAL, "No X windows display available.");
    }
    XVisualInfo *vi;
    static int attributes[] = {GLX_RGBA,
            GLX_RED_SIZE, 1,
            GLX_GREEN_SIZE, 1,
            GLX_BLUE_SIZE, 1,
            0};
    vi = glXChooseVisual(dpy, DefaultScreen(dpy), attributes);
    m_Context = glXCreateContext(dpy, vi, 0, GL_TRUE);
    AVG_ASSERT(m_Context);
    Pixmap pmp = XCreatePixmap(dpy, RootWindow(dpy, vi->screen),
            8, 8, vi->depth);
    GLXPixmap pixmap = glXCreateGLXPixmap(dpy, vi, pmp);
    
    s_bX11Error = false;
    s_DefaultErrorHandler = XSetErrorHandler(X11ErrorHandler);
    glXMakeCurrent(dpy, pixmap, m_Context);
    XSetErrorHandler(s_DefaultErrorHandler);
    
    if (s_bX11Error) {
        throw Exception(AVG_ERR_VIDEO_GENERAL, "X error creating OpenGL context.");
    }
#else
#ifdef _WIN32
    registerWindowClass();
    m_hwnd = CreateWindow("GLUT", "GLUT",
            WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
            0, 0, 500, 300, 0, 0, GetModuleHandle(NULL), 0);
    winOGLErrorCheck(m_hDC != 0, "CreateWindow");
    
    m_hDC = GetDC(m_hwnd);
    winOGLErrorCheck(m_hDC != 0, "GetDC");

    PIXELFORMATDESCRIPTOR pfd;
    ZeroMemory(&pfd, sizeof(pfd));
    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
    pfd.cDepthBits = 32;
    pfd.iLayerType = PFD_MAIN_PLANE;
    
    int iFormat = ChoosePixelFormat(m_hDC, &pfd);
    winOGLErrorCheck(iFormat != 0, "ChoosePixelFormat");
    SetPixelFormat(m_hDC, iFormat, &pfd);
    m_Context = wglCreateContext(m_hDC);
    winOGLErrorCheck(m_Context != 0, "wglCreateContext");

    BOOL bOK = wglMakeCurrent(m_hDC, m_Context);
    winOGLErrorCheck(bOK, "wglMakeCurrent");
#endif
#endif
#endif
    glproc::init();

    if (!isSupported()) {
        throw Exception(AVG_ERR_VIDEO_GENERAL, 
                "GPU imaging not supported by current OpenGL configuration.");
    }

    // Coordinates
    setSize(size);

    setStandardState(size);
//    IteratingGPUFilter f(IntPoint(100, 100), 15);
}

OGLImagingContext::~OGLImagingContext()
{
#ifdef __APPLE__
    if (m_Context) {
        aglSetCurrentContext(0);
        aglDestroyContext(m_Context);
        m_Context = 0;
    }
#endif
#ifdef _WIN32
    if (m_Context) {
        wglDeleteContext(m_Context);
        DeleteDC(m_hDC);
        DestroyWindow(m_hwnd);
    }
#endif
}

void OGLImagingContext::activate()
{
#ifdef __APPLE__
    bool bOk = aglSetCurrentContext(m_Context);
    AVG_ASSERT(bOk);
#elif defined _WIN32
    BOOL bOk = wglMakeCurrent(m_hDC, m_Context);
    AVG_ASSERT(bOk);
#endif
    // TODO: X version
}

void OGLImagingContext::setSize(const IntPoint& size)
{
    m_Size = size;
    setSizeState(size);
}

bool OGLImagingContext::isSupported()
{
    int glMajorVer;
    int glMinorVer;
    int slMajorVer;
    int slMinorVer;
    getGLVersion(glMajorVer, glMinorVer);
    getGLShadingLanguageVersion(slMajorVer, slMinorVer);
    // Not sure if we need shader version 1.2 as well - we'll see.
    return (glMajorVer > 1 && queryOGLExtension("GL_ARB_texture_rectangle") && 
            queryOGLExtension("GL_ARB_pixel_buffer_object") &&
            queryOGLExtension("GL_EXT_framebuffer_object"));
}

void OGLImagingContext::setStandardState(const IntPoint & size)
{
    setSizeState(size);

    // Shading etc.
    glDisable(GL_BLEND);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "glDisable(GL_BLEND)");
    glShadeModel(GL_FLAT);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "glShadeModel(GL_FLAT)");
    glDisable(GL_DEPTH_TEST);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "glDisable(GL_DEPTH_TEST)");
    glDisable(GL_STENCIL_TEST);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "glDisable(GL_STENCIL_TEST)");

    // Texturing
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE); 
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "glTexEnvf()");
    glBlendFunc(GL_ONE, GL_ZERO);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "glBlendFunc()");
    glDisable(GL_MULTISAMPLE);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "glDisable(GL_MULTISAMPLE);");

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
}

void OGLImagingContext::setSizeState(const IntPoint & size)
{
    glViewport(0, 0, size.x, size.y);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "glViewport()");
    glMatrixMode(GL_PROJECTION);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "glMatrixMode()");
    glLoadIdentity();
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "glLoadIdentity()");
    gluOrtho2D(0, size.x, size.y, 0);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "gluOrtho2D()");
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "glLoadIdentity()");
}

}

