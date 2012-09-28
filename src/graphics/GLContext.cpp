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

#include "GLContext.h"

#include "ShaderRegistry.h"
#include "StandardShader.h"

#include "../base/Backtrace.h"
#include "../base/Exception.h"
#include "../base/Logger.h"
#include "../base/MathHelper.h"

#include <SDL/SDL.h>
#include <SDL/SDL_syswm.h>

#include <iostream>


namespace avg {

using namespace std;
using namespace boost;

thread_specific_ptr<GLContext*> GLContext::s_pCurrentContext;
GLContext* GLContext::s_pMainContext = 0; // Optimized access to main context.
bool GLContext::s_bErrorCheckEnabled = false;

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
    pClassName = "GL";

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

void APIENTRY debugLogCallback(GLenum source, GLenum type, GLuint id, GLenum severity,
        GLsizei length, const GLchar* message, void* userParam) 
{
/*    
    string sSource;
    switch (source) {
        case GL_DEBUG_SOURCE_API_ARB:
            sSource = "API";
            break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM_ARB:
            sSource = "Window System";
            break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER_ARB:
            sSource = "Shader Compiler";
            break;
        case GL_DEBUG_SOURCE_THIRD_PARTY_ARB:
            sSource = "Third Party";
            break;
        case GL_DEBUG_SOURCE_APPLICATION_ARB:
            sSource = "Application";
            break;
        case GL_DEBUG_SOURCE_OTHER_ARB:
            sSource = "Other";
            break;
        default:
            AVG_ASSERT(false);
    }

    string sSeverity;
    switch (severity) {
        case GL_DEBUG_SEVERITY_HIGH_ARB:
            sSeverity = "High";
            break;
        case GL_DEBUG_SEVERITY_MEDIUM_ARB:
            sSeverity = "Medium";
            break;
        case GL_DEBUG_SEVERITY_LOW_ARB:
            sSeverity = "Low";
            break;
        default:
            AVG_ASSERT(false);
    }

    string sType;
    switch (type) {
        case GL_DEBUG_TYPE_ERROR_ARB:
            sType = "Error";
            break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR_ARB:
            sType = "Deprecated Behaviour";
            break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR_ARB:
            sType = "Undefined Behaviour";
            break;
        case GL_DEBUG_TYPE_PORTABILITY_ARB:
            sType = "Portability Issue";
            break;
        case GL_DEBUG_TYPE_PERFORMANCE_ARB:
            sType = "Performance Issue";
            break;
        case GL_DEBUG_TYPE_OTHER_ARB:
            sType = "Other";
            break;
        default:
            AVG_ASSERT(false);
    }
*/
    
    AVG_TRACE(Logger::WARNING, message);
//    dumpBacktrace();
}

GLContext::VBMethod GLContext::s_VBMethod = VB_NONE;


GLContext::GLContext(const GLConfig& glConfig, const IntPoint& windowSize, 
        const SDL_SysWMinfo* pSDLWMInfo, GLContext* pSharedContext)
    : m_Context(0),
      m_bOwnsContext(true),
      m_MaxTexSize(0),
      m_bCheckedGPUMemInfoExtension(false),
      m_bCheckedMemoryMode(false),
      m_BlendColor(0.f, 0.f, 0.f, 0.f),
      m_BlendMode(BLEND_ADD)
{
    if (s_pCurrentContext.get() == 0) {
        s_pCurrentContext.reset(new (GLContext*));
    }
    m_GLConfig = glConfig;
#ifdef __APPLE__
    if (pSDLWMInfo) {
        m_Context = CGLGetCurrentContext();
        m_bOwnsContext = false;
        *s_pCurrentContext = this;
    } else {
        CGLPixelFormatObj   pixelFormatObj;
        GLint               numPixelFormats;

        CGLPixelFormatAttribute attribs[] = {(CGLPixelFormatAttribute)NULL};
        CGLContextObj cglSharedContext;
        if (pSharedContext) {
            cglSharedContext = pSharedContext->m_Context;
            pixelFormatObj = CGLGetPixelFormat(cglSharedContext);
        } else {
            cglSharedContext = 0;
            CGLChoosePixelFormat(attribs, &pixelFormatObj, &numPixelFormats);
        }

        CGLError err = CGLCreateContext(pixelFormatObj, cglSharedContext, &m_Context);
        if (err) {
            cerr << CGLErrorString(err) << endl;
            AVG_ASSERT(false);
        }
        CGLDestroyPixelFormat(pixelFormatObj);
    }
#elif defined(__linux__)
    createGLXContext(glConfig, windowSize, pSDLWMInfo);
#elif defined(_WIN32)
    if (pSDLWMInfo) {
        m_hDC = wglGetCurrentDC();
        m_Context = wglGetCurrentContext();
        *s_pCurrentContext = this;
        m_bOwnsContext = false;
    } else {
        registerWindowClass();
        m_hwnd = CreateWindow("GL", "GL",
                WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
                0, 0, 500, 300, 0, 0, GetModuleHandle(NULL), 0);
        checkWinError(m_hwnd != 0, "CreateWindow");

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
        checkWinError(iFormat != 0, "ChoosePixelFormat");
        SetPixelFormat(m_hDC, iFormat, &pfd);
        m_Context = wglCreateContext(m_hDC);
        checkWinError(m_Context != 0, "wglCreateContext");
    }
#endif

    init();
}

#ifdef linux

static bool s_bX11Error;
static int (*s_DefaultErrorHandler) (Display *, XErrorEvent *);

int X11ErrorHandler(Display * pDisplay, XErrorEvent * pErrEvent)
{
    cerr << "X11 error creating GL context: " << (int)(pErrEvent->request_code)
            << ", " << (int)(pErrEvent->minor_code) << endl;
    s_bX11Error = true;
    return 0;
}

void appendGLXVisualAttribute(int* pNumAttributes, int* pAttributes, int newAttr, 
        int newAttrVal=-1)
{
    pAttributes[(*pNumAttributes)++] = newAttr;
    if (newAttrVal != -1) {
        pAttributes[(*pNumAttributes)++] = newAttrVal;
    }
    pAttributes[*pNumAttributes] = 0;
}

bool haveARBCreateContext()
{
    static bool s_bExtensionChecked = false;
    static bool s_bHaveExtension = false;
    if (!s_bExtensionChecked) {
        s_bExtensionChecked = true;
        s_bHaveExtension = (queryGLXExtension("GLX_ARB_create_context"));
    }
    return s_bHaveExtension;
}

void GLContext::createGLXContext(const GLConfig& glConfig, const IntPoint& windowSize, 
        const SDL_SysWMinfo* pSDLWMInfo)
{
    XVisualInfo *pVisualInfo;
    Window win = 0;
    if (pSDLWMInfo) {
        // SDL window exists, use it.
        int attributes[50];
        int numAttributes=0;
        appendGLXVisualAttribute(&numAttributes, attributes, GLX_X_RENDERABLE, 1);
        appendGLXVisualAttribute(&numAttributes, attributes, GLX_DRAWABLE_TYPE, 
                GLX_WINDOW_BIT);
        appendGLXVisualAttribute(&numAttributes, attributes, GLX_RENDER_TYPE, 
                GLX_RGBA_BIT);
        appendGLXVisualAttribute(&numAttributes, attributes, GLX_X_VISUAL_TYPE, 
                GLX_TRUE_COLOR);
        appendGLXVisualAttribute(&numAttributes, attributes, GLX_DEPTH_SIZE, 0);
        appendGLXVisualAttribute(&numAttributes, attributes, GLX_STENCIL_SIZE, 8);
        appendGLXVisualAttribute(&numAttributes, attributes, GLX_DOUBLEBUFFER, 1);
        appendGLXVisualAttribute(&numAttributes, attributes, GLX_RED_SIZE, 8);
        appendGLXVisualAttribute(&numAttributes, attributes, GLX_GREEN_SIZE, 8);
        appendGLXVisualAttribute(&numAttributes, attributes, GLX_BLUE_SIZE, 8);
        appendGLXVisualAttribute(&numAttributes, attributes, GLX_ALPHA_SIZE, 0);

        m_pDisplay = pSDLWMInfo->info.x11.display;
        if (!m_pDisplay) {
            throw Exception(AVG_ERR_VIDEO_GENERAL, "No X windows display available.");
        }
        int fbCount;
        GLXFBConfig* pFBConfig = glXChooseFBConfig(m_pDisplay, DefaultScreen(m_pDisplay), 
                attributes, &fbCount);
        if (!pFBConfig) {
            throw Exception(AVG_ERR_UNSUPPORTED, "Creating OpenGL context failed.");
        }
    
        // Find the config with the appropriate number of multisample samples.
        int bestConfig = -1;
        int bestSamples = -1;
        for (int i=0; i<fbCount; ++i) {
            XVisualInfo* pVisualInfo = glXGetVisualFromFBConfig(m_pDisplay, pFBConfig[i]);
            if (pVisualInfo) {
                int buffer;
                int samples;
                glXGetFBConfigAttrib(m_pDisplay, pFBConfig[i], GLX_SAMPLE_BUFFERS,
                        &buffer);
                glXGetFBConfigAttrib(m_pDisplay, pFBConfig[i], GLX_SAMPLES, &samples);
                if (bestConfig < 0 || 
                        (buffer == 1 && samples > bestSamples && 
                         samples <= glConfig.m_MultiSampleSamples))
                {
                    bestConfig = i;
                    bestSamples = samples;
                }
                XFree(pVisualInfo);
            }
        }
        GLXFBConfig fbConfig = pFBConfig[bestConfig];
        XFree(pFBConfig);
        pVisualInfo = glXGetVisualFromFBConfig(m_pDisplay, fbConfig);

        // Create a child window with the required attributes to render into.
        XSetWindowAttributes swa;
        m_Colormap = XCreateColormap(m_pDisplay, 
                RootWindow(m_pDisplay, pVisualInfo->screen), 
                pVisualInfo->visual, AllocNone);
        swa.colormap = m_Colormap;
        swa.background_pixmap = None;
        swa.event_mask = StructureNotifyMask; 
        win = XCreateWindow(m_pDisplay, pSDLWMInfo->info.x11.window, 
                0, 0, windowSize.x, windowSize.y, 0, pVisualInfo->depth, InputOutput, 
                pVisualInfo->visual, CWColormap|CWEventMask, &swa);
        AVG_ASSERT(win);
        XMapWindow(m_pDisplay, win);
        if (haveARBCreateContext()) {
            int pContextAttribs[50];
            int numContextAttribs = 0;
            pContextAttribs[0] = 0;
            if (m_GLConfig.m_bGLES) {
                appendGLXVisualAttribute(&numContextAttribs, pContextAttribs,
                        GLX_CONTEXT_PROFILE_MASK_ARB, GLX_CONTEXT_ES2_PROFILE_BIT_EXT);
                appendGLXVisualAttribute(&numContextAttribs, pContextAttribs,
                        GLX_CONTEXT_MAJOR_VERSION_ARB, 2);
                appendGLXVisualAttribute(&numContextAttribs, pContextAttribs,
                        GLX_CONTEXT_MINOR_VERSION_ARB, 0);
                if (glConfig.m_bUseDebugContext) {
                    appendGLXVisualAttribute(&numContextAttribs, pContextAttribs,
                            GLX_CONTEXT_FLAGS_ARB, GLX_CONTEXT_DEBUG_BIT_ARB);
                }
            }
            PFNGLXCREATECONTEXTATTRIBSARBPROC CreateContextAttribsARB = 
                    (PFNGLXCREATECONTEXTATTRIBSARBPROC)
                    getglXProcAddress("glXCreateContextAttribsARB");

            m_Context = CreateContextAttribsARB(m_pDisplay, fbConfig, 0,
                    1, pContextAttribs);
        } else {
            m_Context = glXCreateContext(m_pDisplay, pVisualInfo, 0, GL_TRUE);
        }
        *s_pCurrentContext = this;
    } else {
        // Secondary context, no window necessary. Framebuffer is an X pixmap.
        int attributes[] = {GLX_RGBA,
            GLX_RED_SIZE, 1,
            GLX_GREEN_SIZE, 1,
            GLX_BLUE_SIZE, 1,
            0};
        m_pDisplay = XOpenDisplay(0);
        pVisualInfo = glXChooseVisual(m_pDisplay, DefaultScreen(m_pDisplay), attributes);
        if (!pVisualInfo) {
            throw Exception(AVG_ERR_UNSUPPORTED, "Creating OpenGL context failed.");
        }
        m_Context = glXCreateContext(m_pDisplay, pVisualInfo, 0, GL_TRUE);
    }

    AVG_ASSERT(m_Context);
    s_bX11Error = false;
    s_DefaultErrorHandler = XSetErrorHandler(X11ErrorHandler);
    if (pSDLWMInfo) {
        glXMakeCurrent(m_pDisplay, win, m_Context);
    } else { 
        Pixmap pmp = XCreatePixmap(m_pDisplay, 
                RootWindow(m_pDisplay, pVisualInfo->screen), 8, 8, pVisualInfo->depth);
        GLXPixmap pixmap = glXCreateGLXPixmap(m_pDisplay, pVisualInfo, pmp);

        glXMakeCurrent(m_pDisplay, pixmap, m_Context);
    }
    XSetErrorHandler(s_DefaultErrorHandler);

    if (s_bX11Error) {
        throw Exception(AVG_ERR_VIDEO_GENERAL, "X error creating OpenGL context.");
    }
    m_Drawable = glXGetCurrentDrawable();
}
#endif

GLContext::~GLContext()
{
    m_pStandardShader = StandardShaderPtr();
    for (unsigned i=0; i<m_FBOIDs.size(); ++i) {
        glproc::DeleteFramebuffers(1, &(m_FBOIDs[i]));
    }
    m_FBOIDs.clear();
    if (*s_pCurrentContext == this) {
        *s_pCurrentContext = 0;
    }
    if (m_Context && m_bOwnsContext) {
#ifdef __APPLE__
        CGLSetCurrentContext(0);
        CGLDestroyContext(m_Context);
        m_Context = 0;
#elif defined _WIN32
        wglDeleteContext(m_Context);
        DeleteDC(m_hDC);
        DestroyWindow(m_hwnd);
#elif defined __linux__
        glXMakeCurrent(m_pDisplay, 0, 0);
        glXDestroyContext(m_pDisplay, m_Context);
        m_Context = 0;
        XDestroyWindow(m_pDisplay, m_Drawable);
        XFreeColormap(m_pDisplay, m_Colormap);
#endif
    }
}

void GLContext::init()
{
    activate();
    glproc::init();

    if (m_GLConfig.m_bUseDebugContext) {
        if (queryOGLExtension("GL_ARB_debug_output")) {
            glproc::DebugMessageCallback(debugLogCallback, 0);
        } else {
            m_GLConfig.m_bUseDebugContext = false;
        }
    }
    m_pShaderRegistry = ShaderRegistryPtr(new ShaderRegistry());
    if (useGPUYUVConversion()) {
        m_pShaderRegistry->setPreprocessorDefine("ENABLE_YUV_CONVERSION", "");
    }
    glEnableClientState(GL_COLOR_ARRAY);
    checkError("glEnableClientState(GL_COLOR_ARRAY)");
    setBlendMode(BLEND_BLEND, false);
    if (!m_GLConfig.m_bUsePOTTextures) {
        m_GLConfig.m_bUsePOTTextures = 
                !queryOGLExtension("GL_ARB_texture_non_power_of_two");
    }
    if (m_GLConfig.m_ShaderUsage == GLConfig::AUTO) {
        int majorVer;
        int minorVer;
        getGLVersion(majorVer, minorVer);
        if (majorVer > 1) {
            m_GLConfig.m_ShaderUsage = GLConfig::FULL;
        } else {
            m_GLConfig.m_ShaderUsage = GLConfig::MINIMAL;
        }
    }
    for (int i=0; i<16; ++i) {
        m_BoundTextures[i] = 0xFFFFFFFF;
    }
    if (!m_GLConfig.m_bGLES && !queryOGLExtension("GL_ARB_vertex_buffer_object")) {
        throw Exception(AVG_ERR_UNSUPPORTED,
           "Graphics driver lacks vertex buffer support, unable to initialize graphics.");
    }
}

void GLContext::activate()
{
#ifdef __APPLE__
    CGLError err = CGLSetCurrentContext(m_Context);
    AVG_ASSERT(err == kCGLNoError);
#elif defined linux
    glXMakeCurrent(m_pDisplay, m_Drawable, m_Context);
#elif defined _WIN32
    BOOL bOk = wglMakeCurrent(m_hDC, m_Context);
    checkWinError(bOk, "wglMakeCurrent");
#endif
    *s_pCurrentContext = this;
}

ShaderRegistryPtr GLContext::getShaderRegistry() const
{
    return m_pShaderRegistry;
}

StandardShaderPtr GLContext::getStandardShader()
{
    if (m_pStandardShader == StandardShaderPtr()) {
        m_pStandardShader = StandardShaderPtr(new StandardShader());
    }
    return m_pStandardShader;
}

bool GLContext::useGPUYUVConversion() const
{
    int majorVer;
    int minorVer;
    getGLVersion(majorVer, minorVer);
    return (majorVer > 1);
}

bool GLContext::useMinimalShader()
{
    if (m_GLConfig.m_ShaderUsage == GLConfig::FULL) {
        return false;
    } else if (m_GLConfig.m_ShaderUsage == GLConfig::MINIMAL) {
        return true;
    } else {
        AVG_ASSERT(false);
        return false; // Silence compiler warning.
    }
}

GLBufferCache& GLContext::getVertexBufferCache()
{
    return m_VertexBufferCache;
}

GLBufferCache& GLContext::getIndexBufferCache()
{
    return m_IndexBufferCache;
}

GLBufferCache& GLContext::getPBOCache()
{
    return m_PBOCache;
}

unsigned GLContext::genFBO()
{
    unsigned fboID;
    if (m_FBOIDs.empty()) {
        glproc::GenFramebuffers(1, &fboID);
    } else {
        fboID = m_FBOIDs.back();
        m_FBOIDs.pop_back();
    }
    return fboID;
}

void GLContext::returnFBOToCache(unsigned fboID) 
{
    m_FBOIDs.push_back(fboID);
}

void GLContext::setBlendColor(const glm::vec4& color)
{
    if (m_BlendColor != color) {
        glproc::BlendColor(color[0], color[1], color[2], color[3]);
        m_BlendColor = color;
    }
}

void GLContext::setBlendMode(BlendMode mode, bool bPremultipliedAlpha)
{
    GLenum srcFunc;
    if (bPremultipliedAlpha) {
        srcFunc = GL_CONSTANT_ALPHA;
    } else {
        srcFunc = GL_SRC_ALPHA;
    }
    if (mode != m_BlendMode || m_bPremultipliedAlpha != bPremultipliedAlpha) {
        switch (mode) {
            case BLEND_BLEND:
                glproc::BlendEquation(GL_FUNC_ADD);
                glproc::BlendFuncSeparate(srcFunc, GL_ONE_MINUS_SRC_ALPHA, 
                        GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
                checkError("setBlendMode: blend");
                break;
            case BLEND_ADD:
                glproc::BlendEquation(GL_FUNC_ADD);
                glproc::BlendFuncSeparate(srcFunc, GL_ONE, GL_ONE, GL_ONE);
                checkError("setBlendMode: add");
                break;
            case BLEND_MIN:
                glproc::BlendEquation(GL_MIN);
                glproc::BlendFuncSeparate(srcFunc, GL_ONE_MINUS_SRC_ALPHA, 
                        GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
                checkError("setBlendMode: min");
                break;
            case BLEND_MAX:
                glproc::BlendEquation(GL_MAX);
                glproc::BlendFuncSeparate(srcFunc, GL_ONE_MINUS_SRC_ALPHA, 
                        GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
                checkError("setBlendMode: max");
                break;
            case BLEND_COPY:
                glproc::BlendEquation(GL_FUNC_ADD);
                glBlendFunc(GL_ONE, GL_ZERO);
                checkError("setBlendMode: copy");
                break;
            default:
                AVG_ASSERT(false);
        }

        m_BlendMode = mode;
        m_bPremultipliedAlpha = bPremultipliedAlpha;
    }
}

void GLContext::bindTexture(unsigned unit, unsigned texID)
{
    if (m_BoundTextures[unit-GL_TEXTURE0] != texID) {
        glproc::ActiveTexture(unit);
        checkError("GLContext::bindTexture ActiveTexture()");
        glBindTexture(GL_TEXTURE_2D, texID);
        checkError("GLContext::bindTexture BindTexture()");
        m_BoundTextures[unit-GL_TEXTURE0] = texID;
    }
}

const GLConfig& GLContext::getConfig()
{
    return m_GLConfig;
}

void GLContext::logConfig() 
{
    AVG_TRACE(Logger::CONFIG, "OpenGL configuration: ");
    AVG_TRACE(Logger::CONFIG, "  OpenGL version: " << glGetString(GL_VERSION));
    AVG_TRACE(Logger::CONFIG, "  OpenGL vendor: " << glGetString(GL_VENDOR));
    AVG_TRACE(Logger::CONFIG, "  OpenGL renderer: " << glGetString(GL_RENDERER));
    m_GLConfig.log();
    switch (getMemoryModeSupported()) {
        case MM_PBO:
            AVG_TRACE(Logger::CONFIG, "  Using pixel buffer objects");
            break;
        case MM_OGL:
            AVG_TRACE(Logger::CONFIG, "  Not using GL memory extensions");
            break;
    }
    AVG_TRACE(Logger::CONFIG, "  Max. texture size: " << getMaxTexSize());
    string s;
    if (useGPUYUVConversion()) {
        s = "yes";
    } else {
        s = "no";
    }
    AVG_TRACE(Logger::CONFIG, string("  GPU-based YUV-RGB conversion: ")+s+".");
    try {
        AVG_TRACE(Logger::CONFIG, "  Dedicated video memory: " << 
                getVideoMemInstalled()/(1024*1024) << " MB");
        AVG_TRACE(Logger::CONFIG, "  Video memory used at start: " << 
                getVideoMemUsed()/(1024*1024) << " MB");
    } catch (Exception) {
        AVG_TRACE(Logger::CONFIG, "  Dedicated video memory: Unknown");
        AVG_TRACE(Logger::CONFIG, "  Video memory used at start: Unknown");
    }
}

size_t GLContext::getVideoMemInstalled()
{
    checkGPUMemInfoSupport();
    int kbMemInstalled;
    glGetIntegerv(GPU_MEMORY_INFO_DEDICATED_VIDMEM_NVX, &kbMemInstalled);
    return (size_t)kbMemInstalled*1024;
}

size_t GLContext::getVideoMemUsed()
{
    checkGPUMemInfoSupport();
    int kbMemAvailable;
    glGetIntegerv(GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX, &kbMemAvailable);
    return getVideoMemInstalled()-(size_t)kbMemAvailable*1024;
}

bool GLContext::usePOTTextures()
{
    return m_GLConfig.m_bUsePOTTextures;
}

OGLMemoryMode GLContext::getMemoryModeSupported()
{
    if (!m_bCheckedMemoryMode) {
        if ((queryOGLExtension("GL_ARB_pixel_buffer_object") || 
             queryOGLExtension("GL_EXT_pixel_buffer_object")) &&
            m_GLConfig.m_bUsePixelBuffers) 
        {
            m_MemoryMode = MM_PBO;
        } else {
            m_MemoryMode = MM_OGL;
        }
        m_bCheckedMemoryMode = true;
    }
    return m_MemoryMode;
}

int GLContext::getMaxTexSize() 
{
    if (m_MaxTexSize == 0) {
        glGetIntegerv(GL_MAX_TEXTURE_SIZE, &m_MaxTexSize);
    }
    return m_MaxTexSize;
}

bool GLContext::initVBlank(int rate) 
{
    if (rate > 0) {
#ifdef __APPLE__
        initMacVBlank(rate);
        s_VBMethod = VB_APPLE;
#elif defined _WIN32
        if (queryOGLExtension("WGL_EXT_swap_control")) {
            glproc::SwapIntervalEXT(rate);
            s_VBMethod = VB_WIN;
        } else {
            AVG_TRACE(Logger::WARNING,
                    "Windows VBlank setup failed: OpenGL Extension not supported.");
            s_VBMethod = VB_NONE;
        }
#else
        if (getenv("__GL_SYNC_TO_VBLANK") != 0) {
            AVG_TRACE(Logger::WARNING, 
                    "__GL_SYNC_TO_VBLANK set. This interferes with libavg vblank handling.");
            s_VBMethod = VB_NONE;
        } else {
            if (queryGLXExtension("GLX_EXT_swap_control")) {
                s_VBMethod = VB_GLX;
                glproc::SwapIntervalEXT(m_pDisplay, m_Drawable, rate);

            } else {
                AVG_TRACE(Logger::WARNING,
                        "Linux VBlank setup failed: OpenGL Extension not supported.");
                s_VBMethod = VB_NONE;
            }
        }
#endif
    } else {
        switch (s_VBMethod) {
            case VB_APPLE:
                initMacVBlank(0);
                break;
            case VB_WIN:
#ifdef _WIN32
                glproc::SwapIntervalEXT(0);
#endif
                break;
            case VB_GLX:
#ifdef linux            
                glproc::SwapIntervalEXT(m_pDisplay, m_Drawable, 0);
#endif
                break;
            default:
                break;
        }
        s_VBMethod = VB_NONE;
    }
    switch(s_VBMethod) {
        case VB_GLX:
            AVG_TRACE(Logger::CONFIG, 
                    "  Using SGI OpenGL extension for vertical blank support.");
            break;
        case VB_APPLE:
            AVG_TRACE(Logger::CONFIG, "  Using Apple GL vertical blank support.");
            break;
        case VB_WIN:
            AVG_TRACE(Logger::CONFIG, "  Using Windows GL vertical blank support.");
            break;
        case VB_NONE:
            AVG_TRACE(Logger::CONFIG, "  Vertical blank support disabled.");
            break;
        default:
            AVG_TRACE(Logger::WARNING, "  Illegal vblank enum value.");
    }
    return s_VBMethod != VB_NONE;
}

void GLContext::swapBuffers()
{
#ifdef linux
    glXSwapBuffers(m_pDisplay, m_Drawable);
#else
    AVG_ASSERT(false);
#endif
}

void GLContext::enableErrorChecks(bool bEnable)
{
    s_bErrorCheckEnabled = bEnable;
}
    
void GLContext::checkError(const char* pszWhere) 
{
    if (s_bErrorCheckEnabled) {
        mandatoryCheckError(pszWhere);
    }
}

void GLContext::mandatoryCheckError(const char* pszWhere) 
{
    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        stringstream s;
        s << "OpenGL error in " << pszWhere <<": " << gluErrorString(err) 
            << " (#" << err << ") ";
        AVG_TRACE(Logger::ERROR, s.str());
        if (err != GL_INVALID_OPERATION) {
            checkError("  --");
        }
        AVG_ASSERT(false);
    }
}

GLContext::BlendMode GLContext::stringToBlendMode(const string& s)
{
    if (s == "blend") {
        return GLContext::BLEND_BLEND;
    } else if (s == "add") {
        return GLContext::BLEND_ADD;
    } else if (s == "min") {
        return GLContext::BLEND_MIN;
    } else if (s == "max") {
        return GLContext::BLEND_MAX;
    } else {
        throw(Exception(AVG_ERR_UNSUPPORTED, "Blend mode "+s+" not supported."));
    }
}

GLContext* GLContext::getCurrent()
{
    return *s_pCurrentContext;
}

GLContext* GLContext::getMain()
{
    return s_pMainContext;
}

void GLContext::setMain(GLContext * pMainContext)
{
    s_pMainContext = pMainContext;
}

int GLContext::nextMultiSampleValue(int curSamples)
{
    switch (curSamples) {
        case 1:
            return 0;
        case 2:  
            return 1;
        case 4:  
            return 2;
        case 8:  
            return 4;
        default:
            return 8;
    }
}

void GLContext::checkGPUMemInfoSupport()
{
    if (!m_bCheckedGPUMemInfoExtension) {
        m_bGPUMemInfoSupported = queryOGLExtension("GL_NVX_gpu_memory_info");
        m_bCheckedGPUMemInfoExtension = true;
    }
    if (!m_bGPUMemInfoSupported) {
        throw Exception(AVG_ERR_UNSUPPORTED, 
                "Video memory query not supported on this system.");
    }
}

#ifdef _WIN32
void GLContext::checkWinError(BOOL bOK, const string& sWhere) 
{
    if (!bOK) {
        char szErr[512];
        FormatMessage((FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM),
                0, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                szErr, 512, 0);
        AVG_TRACE(Logger::ERROR, sWhere+":"+szErr);
        AVG_ASSERT(false);
    }
}
#endif

void GLContext::initMacVBlank(int rate)
{
#ifdef __APPLE__
    CGLContextObj context = CGLGetCurrentContext();
    AVG_ASSERT (context);
#if MAC_OS_X_VERSION_10_5
    GLint l = rate;
#else
    long l = rate;
#endif
    if (rate > 1) {
        AVG_TRACE(Logger::WARNING,
                "VBlank rate set to " << rate 
                << " but Mac OS X only supports 1. Assuming 1.");
        l = 1;
    }
    CGLError err = CGLSetParameter(context, kCGLCPSwapInterval, &l);
    AVG_ASSERT(!err);
#endif
}

}
