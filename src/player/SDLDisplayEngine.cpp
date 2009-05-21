
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


#include "SDLDisplayEngine.h"
#ifdef __APPLE__
#include "SDLMain.h"
#endif

#include "Player.h"
#include "Node.h"
#include "AVGNode.h"
#include "Shape.h"

#include "Event.h"
#include "MouseEvent.h"
#include "KeyEvent.h"

#include "../base/MathHelper.h"
#include "../base/Exception.h"
#include "../base/Logger.h"
#include "../base/ScopeTimer.h"
#include "../base/Profiler.h"
#include "../base/OSHelper.h"

#include "../graphics/Filterflip.h"
#include "../graphics/Filterfliprgb.h"

#ifdef __APPLE__
#include <ApplicationServices/ApplicationServices.h>
#endif
#ifdef linux
#include <X11/extensions/xf86vmode.h>
#endif

#ifdef __APPLE__
#include <OpenGL/OpenGL.h>
#endif

#ifdef linux
#include <sys/ioctl.h>
#include <sys/fcntl.h>
#endif

#include <signal.h>
#include <iostream>
#include <sstream>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#include <assert.h>

using namespace std;

namespace avg {

double SDLDisplayEngine::s_RefreshRate = 0.0;

void dumpSDLGLParams() {
    int value;
    cerr << "SDL display parameters used: "<< endl;
    SDL_GL_GetAttribute(SDL_GL_RED_SIZE, &value);
    fprintf(stderr,"  SDL_GL_RED_SIZE = %d\n",value);
    SDL_GL_GetAttribute(SDL_GL_GREEN_SIZE,  &value);
    fprintf(stderr,"  SDL_GL_GREEN_SIZE = %d\n",value);
    SDL_GL_GetAttribute(SDL_GL_BLUE_SIZE , &value);
    fprintf(stderr,"  SDL_GL_BLUE_SIZE = %d\n",value);
    SDL_GL_GetAttribute(SDL_GL_ALPHA_SIZE , &value);
    fprintf(stderr,"  SDL_GL_ALPHA_SIZE = %d\n",value);
    SDL_GL_GetAttribute(SDL_GL_DEPTH_SIZE , &value);
    fprintf(stderr,"  SDL_GL_DEPTH_SIZE = %d\n",value);
    SDL_GL_GetAttribute(SDL_GL_DOUBLEBUFFER , &value);
    fprintf(stderr,"  SDL_GL_DOUBLEBUFFER = %d\n",value);
    SDL_GL_GetAttribute(SDL_GL_BUFFER_SIZE , &value);
    fprintf(stderr,"  SDL_GL_BUFFER_SIZE = %d\n",value);
    SDL_GL_GetAttribute(SDL_GL_STENCIL_SIZE , &value);
    fprintf(stderr,"  SDL_GL_STENCIL_SIZE = %d\n",value);
}

SDLDisplayEngine::SDLDisplayEngine()
    : m_WindowWidth(0),
      m_WindowHeight(0),
      m_bEnableCrop(false),
      m_pScreen(0),
      m_VBMethod(VB_NONE),
      m_VBMod(0),
      m_bMouseOverApp(true),
      m_LastMousePos(-1, -1),
      m_MaxTexSize(0),
      m_bCheckedMemoryMode(false)
{
#ifdef __APPLE__
    static bool bSDLInitialized = false;
    if (!bSDLInitialized) {
        CustomSDLMain();
        bSDLInitialized = true;
    }
#endif
    if (SDL_InitSubSystem(SDL_INIT_VIDEO)==-1) {
        AVG_TRACE(Logger::ERROR, "Can't init SDL display subsystem.");
        exit(-1);
    }
    initTranslationTable();
    SDL_EnableUNICODE(true);
}

SDLDisplayEngine::~SDLDisplayEngine()
{
#ifndef _WIN32
    SDL_QuitSubSystem(SDL_INIT_VIDEO);
#endif
}

void SDLDisplayEngine::init(const DisplayParams& DP) 
{
    stringstream ss;
    if (DP.m_x != -1) {
        ss << DP.m_x << "," << DP.m_y;
        setEnv("SDL_VIDEO_WINDOW_POS", ss.str().c_str());
    }
#ifdef linux
    int oldWindowWidth = m_WindowWidth;
    int oldWindowHeight = m_WindowHeight;
#endif
    double AspectRatio = double(DP.m_Width)/double(DP.m_Height);
    if (DP.m_WindowWidth == 0 && DP.m_WindowHeight == 0) {
        m_WindowWidth = DP.m_Width;
        m_WindowHeight = DP.m_Height;
    } else if (DP.m_WindowWidth == 0) {
        m_WindowWidth = int(DP.m_WindowHeight*AspectRatio);
        m_WindowHeight = DP.m_WindowHeight;
    } else {
        m_WindowWidth = DP.m_WindowWidth;
        m_WindowHeight = int(DP.m_WindowWidth/AspectRatio);
    }
#ifdef linux
    // Workaround for what appears to be an SDL or X11 bug:
    // Stencil buffers stop working after a window resize, so we reinit
    // everything.
    if (oldWindowWidth != m_WindowWidth || oldWindowHeight != m_WindowHeight) {
        SDL_QuitSubSystem(SDL_INIT_VIDEO);
        if (SDL_InitSubSystem(SDL_INIT_VIDEO)==-1) {
            AVG_TRACE(Logger::ERROR, "Can't init SDL display subsystem.");
            exit(-1);
        }
    }
#endif

    switch (DP.m_BPP) {
        case 32:
            safeSetAttribute( SDL_GL_RED_SIZE, 8 );
            safeSetAttribute( SDL_GL_GREEN_SIZE, 8 );
            safeSetAttribute( SDL_GL_BLUE_SIZE, 8 );
            safeSetAttribute( SDL_GL_BUFFER_SIZE, 32 );
            break;
        case 24:
            safeSetAttribute( SDL_GL_RED_SIZE, 8 );
            safeSetAttribute( SDL_GL_GREEN_SIZE, 8 );
            safeSetAttribute( SDL_GL_BLUE_SIZE, 8 );
            safeSetAttribute( SDL_GL_BUFFER_SIZE, 24 );
            break;
        case 16:
            safeSetAttribute( SDL_GL_RED_SIZE, 5 );
            safeSetAttribute( SDL_GL_GREEN_SIZE, 6 );
            safeSetAttribute( SDL_GL_BLUE_SIZE, 5 );
            safeSetAttribute( SDL_GL_BUFFER_SIZE, 16 );
            break;
        case 15:
            safeSetAttribute( SDL_GL_RED_SIZE, 5 );
            safeSetAttribute( SDL_GL_GREEN_SIZE, 5 );
            safeSetAttribute( SDL_GL_BLUE_SIZE, 5 );
            safeSetAttribute( SDL_GL_BUFFER_SIZE, 15 );
            break;
        default:
            AVG_TRACE(Logger::ERROR, "Unsupported bpp " << DP.m_BPP <<
                    "in SDLDisplayEngine::init()");
            exit(-1);
    }
    safeSetAttribute(SDL_GL_DEPTH_SIZE, 24);
    safeSetAttribute(SDL_GL_STENCIL_SIZE, 8);
    safeSetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    if (m_MultiSampleSamples > 1) {
        safeSetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
        safeSetAttribute(SDL_GL_MULTISAMPLESAMPLES, m_MultiSampleSamples);
    } else {
        safeSetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);
    }

    unsigned int Flags = SDL_OPENGL;
    if (DP.m_bFullscreen) {
        Flags |= SDL_FULLSCREEN;
    }
    m_pScreen = SDL_SetVideoMode(m_WindowWidth, m_WindowHeight, DP.m_BPP, Flags);
    if (!m_pScreen) {
        AVG_TRACE(Logger::ERROR, "Setting SDL video mode failed: " 
                << SDL_GetError() <<". (width=" << m_WindowWidth << ", height=" 
                << m_WindowHeight << ", bpp=" << DP.m_BPP << ", multisamplesamples="
                << m_MultiSampleSamples << ").");
        exit(-1);
    }
    glproc::init();

    SDL_WM_SetCaption("AVG Renderer", 0);
    calcRefreshRate();

    glEnable(GL_BLEND);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "init: glEnable(GL_BLEND)");
    glShadeModel(GL_FLAT);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "init: glShadeModel(GL_FLAT)");
    glDisable(GL_DEPTH_TEST);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "init: glDisable(GL_DEPTH_TEST)");
    glEnable(GL_STENCIL_TEST);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "init: glEnable(GL_STENCIL_TEST)");
    initTextureMode();
    if (!queryOGLExtension("GL_ARB_multisample")) {
        m_MultiSampleSamples = 1;
    } else {
        if (m_MultiSampleSamples > 1) {
            glEnable(GL_MULTISAMPLE);
            OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "init: glEnable(GL_MULTISAMPLE);");
        } else {
            glDisable(GL_MULTISAMPLE);
            OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "init: glDisable(GL_MULTISAMPLE);");
        }
    }
    if (!queryOGLExtension("GL_ARB_vertex_buffer_object")) {
        throw Exception(AVG_ERR_UNSUPPORTED,
            "Graphics driver lacks vertex buffer support, unable to initialize graphics.");
    }
    m_bEnableTexture=false;
    enableTexture(true);
    m_bEnableGLColorArray=true;
    enableGLColorArray(false);
    m_BlendMode = BLEND_ADD;
    setBlendMode(BLEND_BLEND);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    setGamma(DP.m_Gamma[0], DP.m_Gamma[1], DP.m_Gamma[2]);
    showCursor(DP.m_bShowCursor);
    if (DP.m_Framerate == 0) {
        setVBlankRate(DP.m_VBRate);
    } else {
        setFramerate(DP.m_Framerate);
    }

    checkYCbCrSupport();

    m_Width = DP.m_Width;
    m_Height = DP.m_Height;
    initInput();
    // SDL sets up a signal handler we really don't want.
    signal(SIGSEGV, SIG_DFL);
    logConfig();

//    dumpSDLGLParams();
    m_bEnableCrop = false;
}

#ifdef _WIN32
#pragma warning(disable: 4996)
#endif
void SDLDisplayEngine::teardown()
{
    if (m_pScreen) {
#ifdef linux
        // Workaround for broken mouse cursor on exit under Ubuntu 8.04.
        SDL_ShowCursor(SDL_ENABLE);
//        SDL_SetVideoMode(m_WindowWidth, m_WindowHeight, 24, 0);
#endif
        m_pScreen = 0;
    }
    VertexArray::deleteBufferCache();
}

double SDLDisplayEngine::getRefreshRate() 
{
    if (s_RefreshRate == 0.0) {
        calcRefreshRate();
    }
    return s_RefreshRate;
}

void SDLDisplayEngine::setGamma(double Red, double Green, double Blue)
{
    if (Red > 0) {
        AVG_TRACE(Logger::CONFIG, "Setting gamma to " << Red << ", " << Green << ", " << Blue);
        int err = SDL_SetGamma(float(Red), float(Green), float(Blue));
        if (err == -1) {
            AVG_TRACE(Logger::WARNING, "Unable to set display gamma.");
        }
    }
}

void SDLDisplayEngine::setMousePos(const IntPoint& pos)
{
    SDL_WarpMouse(pos.x, pos.y);
}

void SDLDisplayEngine::logConfig() 
{
    AVG_TRACE(Logger::CONFIG, "OpenGL configuration: ");
    AVG_TRACE(Logger::CONFIG, "  OpenGL version: " << glGetString(GL_VERSION));
    AVG_TRACE(Logger::CONFIG, "  OpenGL vendor: " << glGetString(GL_VENDOR));
    AVG_TRACE(Logger::CONFIG, "  OpenGL renderer: " << glGetString(GL_RENDERER));
    if (m_bUseYCbCrShaders) {
        AVG_TRACE(Logger::CONFIG, "  Using fragment shader YCbCr texture support.");
    } else {
        AVG_TRACE(Logger::CONFIG, "  YCbCr texture support not enabled.");
    }
    if (m_bUsePOTTextures) {
        AVG_TRACE(Logger::CONFIG, "  Using power of 2 textures.");
    } else {
        AVG_TRACE(Logger::CONFIG, "  Using non-power of 2 textures.");
    }
    if (m_MultiSampleSamples == 1) {
        AVG_TRACE(Logger::CONFIG, "  Not using multisampling.");
    } else {
        AVG_TRACE(Logger::CONFIG, "  Using multisampling with " << m_MultiSampleSamples 
                << " samples");
    }
    switch (getMemoryModeSupported()) {
        case PBO:
            AVG_TRACE(Logger::CONFIG, "  Using pixel buffer objects.");
            break;
        case OGL:
            AVG_TRACE(Logger::CONFIG, "  Not using GL memory extensions.");
            break;
    }
        AVG_TRACE(Logger::CONFIG,
                "  Max. texture size is " << getMaxTexSize());
}

static ProfilingZone RootRenderProfilingZone("Root node: render");

void SDLDisplayEngine::render(AVGNodePtr pRootNode)
{
    pRootNode->preRender();
    m_bEnableCrop = pRootNode->getCropSetting();
    
    glClearColor(0.0, 0.0, 0.0, 0.0); 
    glClear(GL_COLOR_BUFFER_BIT);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            "SDLDisplayEngine::render::glClear(GL_COLOR_BUFFER_BIT)");
    glClearStencil(0);
    glClear(GL_STENCIL_BUFFER_BIT);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            "SDLDisplayEngine::render::glClear(GL_STENCIL_BUFFER_BIT)");
    glClear(GL_DEPTH_BUFFER_BIT);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            "SDLDisplayEngine::render::glClear(GL_DEPTH_BUFFER_BIT)");
    glViewport(0, 0, m_WindowWidth, m_WindowHeight);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            "SDLDisplayEngine::render: glViewport()");
    glMatrixMode(GL_PROJECTION);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            "SDLDisplayEngine::render: glMatrixMode()");
    glLoadIdentity();
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            "SDLDisplayEngine::render: glLoadIdentity()");
    gluOrtho2D(0, m_Width, m_Height, 0);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            "SDLDisplayEngine::render: gluOrtho2D()");
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE); 
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            "SDLDisplayEngine::render: glTexEnvf()");
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            "SDLDisplayEngine::render: glBlendFunc()");
    
    const DRect rc(0,0, m_Width, m_Height);
    glMatrixMode(GL_MODELVIEW);
    {
        ScopeTimer Timer(RootRenderProfilingZone);
        pRootNode->maybeRender(rc);

        Shape * pShape = new Shape("", GL_REPEAT, GL_CLAMP_TO_EDGE);
        pShape->moveToGPU(this);
        VertexArrayPtr pVA = pShape->getVertexArray();
        pVA->changeSize(8000, 8000);  // Breaks at 1000 nodes.
        pRootNode->renderOutlines(pVA, Pixel32(0,0,0,0));
        if (pVA->getCurVert() != 0) {
            pVA->changeSize(pVA->getCurVert(), pVA->getCurIndex());
            pVA->update();
            pShape->draw();
        }
        delete pShape;
    }
    frameWait();
    swapBuffers();
    checkJitter();
}

static ProfilingZone PushClipRectProfilingZone("pushClipRect");

bool SDLDisplayEngine::pushClipRect(const DRect& rc)
{
    ScopeTimer Timer(PushClipRectProfilingZone);

    m_ClipRects.push_back(rc);
    clip(true);

    return true;
}

static ProfilingZone PopClipRectProfilingZone("popClipRect");

void SDLDisplayEngine::popClipRect()
{
    ScopeTimer Timer(PopClipRectProfilingZone);
    
    clip(false);
    m_ClipRects.pop_back();
}

const DRect& SDLDisplayEngine::getClipRect() 
{
    return m_ClipRects.back();
}

void SDLDisplayEngine::pushTransform(const DPoint& translate, double angle, const DPoint& pivot)
{
    glPushMatrix();
    glTranslated(translate.x, translate.y, 0);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "pushTransform: glTranslated");
    glTranslated(pivot.x, pivot.y, 0);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "pushTransform: glTranslated");
    glRotated(angle*180.0/PI, 0, 0, 1);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "pushTransform: glRotated");
    glTranslated(-pivot.x, -pivot.y, 0);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "pushTransform: glTranslated");
}

void SDLDisplayEngine::popTransform()
{
    glPopMatrix();
}

void SDLDisplayEngine::clip(bool forward)
{
    if (m_bEnableCrop && !m_ClipRects.empty()) {
        GLenum stencilOp;
        int level;
        if(forward) {
            stencilOp = GL_INCR;
            level = int(m_ClipRects.size());
        } else {
            stencilOp = GL_DECR;
            level = int(m_ClipRects.size()) - 1;
        }
        
        DRect rc = m_ClipRects.back();
        
        // Disable drawing to color buffer
        glColorMask(0, 0, 0, 0);
        
        // Enable drawing to stencil buffer
        glStencilMask(~0);

        // Draw clip rectangle into stencil buffer
        glStencilFunc(GL_ALWAYS, 0, 0);
        glStencilOp(stencilOp, stencilOp, stencilOp);

        glBegin(GL_QUADS);
            glVertex2d(rc.tl.x, rc.tl.y);
            glVertex2d(rc.br.x, rc.tl.y);
            glVertex2d(rc.br.x, rc.br.y);
            glVertex2d(rc.tl.x, rc.br.y);
        glEnd();

        // Set stencil test to only let
        glStencilFunc(GL_LEQUAL, level, ~0);
        glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
        
        // Disable drawing to stencil buffer
        glStencilMask(0);
        
        // Enable drawing to color buffer
        glColorMask(~0, ~0, ~0, ~0);
    }
}

static ProfilingZone SwapBufferProfilingZone("Render - swap buffers");

void SDLDisplayEngine::swapBuffers()
{
    ScopeTimer Timer(SwapBufferProfilingZone);
    SDL_GL_SwapBuffers();
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "swapBuffers()");
    AVG_TRACE(Logger::BLTS, "GL SwapBuffers");
}

bool SDLDisplayEngine::supportsBpp(int bpp)
{
    return (bpp == 24 || bpp == 32);
}

bool SDLDisplayEngine::hasRGBOrdering()
{
    return false;
}

bool SDLDisplayEngine::isUsingYCbCrShaders()
{
    return m_bUseYCbCrShaders;
}

OGLShaderPtr SDLDisplayEngine::getYCbCr420pShader()
{
    return m_pYCbCrShader;
}

OGLShaderPtr SDLDisplayEngine::getYCbCrJ420pShader()
{
    return m_pYCbCrJShader;
}

void SDLDisplayEngine::showCursor(bool bShow)
{
#ifdef _WIN32
    ShowCursor(bShow);
#else
    if (bShow) {
        SDL_ShowCursor(SDL_ENABLE);
    } else {
        SDL_ShowCursor(SDL_DISABLE);
    }
#endif
}

BitmapPtr SDLDisplayEngine::screenshot()
{
    
    BitmapPtr pBmp (new Bitmap(IntPoint(m_Width, m_Height), R8G8B8X8, "screenshot"));
    if (isParallels()) {
        // Workaround for buggy GL_FRONT on virtual machines running under parallels.
        glReadBuffer(GL_BACK);
    } else {
        glReadBuffer(GL_FRONT);
    }
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "SDLDisplayEngine::screenshot:glReadBuffer()");
    glReadPixels(0, 0, m_Width, m_Height, GL_RGBA, GL_UNSIGNED_BYTE, 
            pBmp->getPixels());
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "SDLDisplayEngine::screenshot:glReadPixels()");
//    FilterFlipRGB().applyInPlace(pBmp);
    FilterFlip().applyInPlace(pBmp);
    return pBmp;
}

int SDLDisplayEngine::getWidth()
{
    return m_Width;
}

int SDLDisplayEngine::getHeight()
{
    return m_Height;
}

int SDLDisplayEngine::getBPP()
{
    return m_bpp;
}
    
void SDLDisplayEngine::checkYCbCrSupport()
{
    if (queryOGLExtension("GL_ARB_fragment_shader") &&
        queryOGLExtension("GL_ARB_texture_rectangle") &&
        getMemoryModeSupported() == PBO &&
        !m_bUsePOTTextures
       )
    {
        m_bUseYCbCrShaders = true;
        string sProgramInit =
            "uniform sampler2D YTexture;\n"
            "uniform sampler2D CbTexture;\n"
            "uniform sampler2D CrTexture;\n"
            "\n";
        string sProgram =
            sProgramInit + 
            "void main(void)\n"
            "{\n"
            "  vec3 YCbCr;\n"
            "  YCbCr.r = texture2D(YTexture, gl_TexCoord[0].st).a-0.0625;\n"
            "  YCbCr.g = texture2D(CbTexture, (gl_TexCoord[0].st)).a-0.5;\n"
            "  YCbCr.b = texture2D(CrTexture, (gl_TexCoord[0].st)).a-0.5;\n"
            "  vec3 RGB;"
            "  RGB = YCbCr*mat3(1.16,  0.0,   1.60,\n"
            "                   1.16, -0.39, -0.81,\n"
            "                   1.16,  2.01,  0.0 );\n"
            "  gl_FragColor = vec4(RGB,gl_Color.a);\n"
            "}\n"
            ;
        m_pYCbCrShader = OGLShaderPtr(new OGLShader(sProgram));

        sProgram =
            sProgramInit + 
            "void main(void)\n"
            "{\n"
            "  vec3 YCbCr;\n"
            "  YCbCr.r = texture2D(YTexture, gl_TexCoord[0].st).a;\n"
            "  YCbCr.g = texture2D(CbTexture, (gl_TexCoord[0].st)).a-0.5;\n"
            "  YCbCr.b = texture2D(CrTexture, (gl_TexCoord[0].st)).a-0.5;\n"
            "  vec3 RGB;"
            "  RGB = YCbCr*mat3(1,  0.0  , 1.40,\n"
            "                   1, -0.34, -0.71,\n"
            "                   1,  1.77,  0.0 );\n"
            "  gl_FragColor = vec4(RGB,gl_Color.a);\n"
            "}\n"
            ;
        m_pYCbCrJShader = OGLShaderPtr(new OGLShader(sProgram));
    } else {
        m_bUseYCbCrShaders = false;
    }
}

bool SDLDisplayEngine::initVBlank(int rate) {
    
    if (rate > 0 && m_DesiredVSyncMode != VSYNC_NONE) {
#ifdef __APPLE__
        CGLContextObj Context = CGLGetCurrentContext();
        if (Context == 0) {
            AVG_TRACE(Logger::WARNING,
                    "Mac VBlank setup failed in CGLGetCurrentContext().");
        }
        if (rate != 1) {
            AVG_TRACE(Logger::WARNING,
                    "VBlank rate set to " << rate << " but Mac OS X only supports 1. Assuming 1.");
        }
#if MAC_OS_X_VERSION_10_5
        const GLint l = 1;
#else
        const long l = 1;
#endif
        CGLError err = CGLSetParameter(Context, kCGLCPSwapInterval, &l);
        m_VBMethod = VB_APPLE;
        if (err) {
            AVG_TRACE(Logger::WARNING,
                    "Mac VBlank setup failed with error code " << 
                    CGLErrorString(err) << "(" << err << ").");
            m_VBMethod = VB_NONE;
        }
#elif defined _WIN32
        if (queryOGLExtension("WGL_EXT_swap_control")) {
            glproc::SwapIntervalEXT(rate);
            m_VBMethod = VB_WIN;
        } else {
            AVG_TRACE(Logger::WARNING,
                    "Windows VBlank setup failed: OpenGL Extension not supported.");
            m_VBMethod = VB_NONE;
        }
#else
        if (getenv("__GL_SYNC_TO_VBLANK") != 0) 
        {
            AVG_TRACE(Logger::WARNING, 
                    "__GL_SYNC_TO_VBLANK set. This interferes with libavg vblank handling.");
            m_VBMethod = VB_NONE;
        } else {
            m_VBMethod = VB_SGI;
            m_bFirstVBFrame = true;
        }
#endif
    } else {
        m_VBMethod = VB_NONE;
    }
    switch(m_VBMethod) {
        case VB_SGI:
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
    return m_VBMethod != VB_NONE;
}

bool SDLDisplayEngine::vbWait(int rate) {
    switch(m_VBMethod) {
#ifdef linux
        case VB_SGI: {
                unsigned int count;
                int err = glproc::WaitVideoSyncSGI(rate, m_VBMod, &count);
                OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
                        "VBlank::glXWaitVideoSyncSGI");
                if (err) {
                    AVG_TRACE(Logger::ERROR, "glXWaitVideoSyncSGI returned " 
                            << err << ".");
                    AVG_TRACE(Logger::ERROR, "Rate was " << rate 
                            << ", Mod was " << m_VBMod);
                    AVG_TRACE(Logger::ERROR, "Aborting.");
                    assert(false);
                }
                m_VBMod = count % rate;
                bool bMissed;
                if (!m_bFirstVBFrame && int(count) != m_LastVBCount+rate) {
                    AVG_TRACE(Logger::PROFILE_LATEFRAMES, count-m_LastVBCount
                            << " VBlank intervals missed, should be " 
                            << rate);
                    bMissed = true;
                } else {
                    bMissed = false;
                }
                m_LastVBCount = count;
                m_bFirstVBFrame = false;
                return !bMissed;
            }
            break;
#endif
        case VB_APPLE:
        case VB_WIN:
            return true;
        case VB_NONE:
        default:
            assert(false);
            return false;
    }
}

void SDLDisplayEngine::calcRefreshRate() {
    double lastRefreshRate = s_RefreshRate;
    s_RefreshRate = 0;
#ifdef __APPLE__
    CFDictionaryRef modeInfo = CGDisplayCurrentMode(CGMainDisplayID());
    if (modeInfo) {
        CFNumberRef value = (CFNumberRef) CFDictionaryGetValue(modeInfo, 
                kCGDisplayRefreshRate);
        if (value) {
            CFNumberGetValue(value, kCFNumberIntType, &s_RefreshRate);
            if (s_RefreshRate < 1.0) {
                AVG_TRACE(Logger::CONFIG, 
                        "This seems to be a TFT screen, assuming 60 Hz refresh rate.");
                s_RefreshRate = 60;
            }
        } else {
            AVG_TRACE(Logger::WARNING, 
                    "Apple refresh rate calculation (CFDictionaryGetValue) failed");
        }
    } else {
        AVG_TRACE(Logger::WARNING, 
                "Apple refresh rate calculation (CGDisplayCurrentMode) failed");
    }
#elif defined _WIN32
     // This isn't correct for multi-monitor systems.
    HDC hDC = CreateDC("DISPLAY", NULL,NULL,NULL);
    s_RefreshRate = GetDeviceCaps(hDC, VREFRESH);
    if (s_RefreshRate < 2) {
        s_RefreshRate = 60;
    }
    DeleteDC(hDC);
#else 
    Display * display = XOpenDisplay(0);
    int PixelClock;
    XF86VidModeModeLine mode_line;
    bool bOK = XF86VidModeGetModeLine (display, DefaultScreen(display), 
            &PixelClock, &mode_line);
    if (!bOK) {
        AVG_TRACE (Logger::WARNING, 
                "Could not get current refresh rate (XF86VidModeGetModeLine failed).");
        AVG_TRACE (Logger::WARNING, 
                "Defaulting to 60 Hz refresh rate.");
    }
    double HSyncRate = PixelClock*1000.0/mode_line.htotal;
    s_RefreshRate = HSyncRate/mode_line.vtotal;
    XCloseDisplay(display);
#endif
    if (s_RefreshRate == 0) {
        s_RefreshRate = 60;
        AVG_TRACE (Logger::WARNING, "Assuming 60 Hz refresh rate.");
    }
    if (lastRefreshRate != s_RefreshRate) {
        AVG_TRACE(Logger::CONFIG, "Vertical Refresh Rate: " << s_RefreshRate);
    }

}

vector<long> SDLDisplayEngine::KeyCodeTranslationTable(SDLK_LAST, key::KEY_UNKNOWN);

void SDLDisplayEngine::initInput()
{
    initJoysticks();
}

const char * getEventTypeName(unsigned char Type) 
{
    switch(Type) {
            case SDL_ACTIVEEVENT:
                return "SDL_ACTIVEEVENT";
            case SDL_KEYDOWN:
                return "SDL_KEYDOWN";
            case SDL_KEYUP:
                return "SDL_KEYUP";
            case SDL_MOUSEMOTION:
                return "SDL_MOUSEMOTION";
            case SDL_MOUSEBUTTONDOWN:
                return "SDL_MOUSEBUTTONDOWN";
            case SDL_MOUSEBUTTONUP:
                return "SDL_MOUSEBUTTONUP";
            case SDL_JOYAXISMOTION:
                return "SDL_JOYAXISMOTION";
            case SDL_JOYBUTTONDOWN:
                return "SDL_JOYBUTTONDOWN";
            case SDL_JOYBUTTONUP:
                return "SDL_JOYBUTTONUP";
            case SDL_VIDEORESIZE:
                return "SDL_VIDEORESIZE";
            case SDL_VIDEOEXPOSE:
                return "SDL_VIDEOEXPOSE";
            case SDL_QUIT:
                return "SDL_QUIT";
            case SDL_USEREVENT:
                return "SDL_USEREVENT";
            case SDL_SYSWMEVENT:
                return "SDL_SYSWMEVENT";
            default:
                return "Unknown SDL event type";
    }
}

vector<EventPtr> SDLDisplayEngine::pollEvents()
{
    SDL_Event sdlEvent;
    vector<EventPtr> Events;

    while(SDL_PollEvent(&sdlEvent)) {
        switch(sdlEvent.type) {
            case SDL_ACTIVEEVENT:
                if (sdlEvent.active.state & SDL_APPMOUSEFOCUS) {
                    m_bMouseOverApp = sdlEvent.active.gain != 0;
                    if (!sdlEvent.active.gain) {
                        Events.push_back(EventPtr(
                                new MouseEvent (Event::CURSORMOTION, false,
                                        false, false, IntPoint(-1, -1), 
                                        MouseEvent::NO_BUTTON, DPoint(0,0))));
                    }
                }
            case SDL_MOUSEMOTION:
                if (m_bMouseOverApp) {
                    Events.push_back
                            (createMouseEvent(Event::CURSORMOTION, sdlEvent, MouseEvent::NO_BUTTON));
                }
                break;
            case SDL_MOUSEBUTTONDOWN:
                Events.push_back
                        (createMouseButtonEvent(Event::CURSORDOWN, sdlEvent));
                break;
            case SDL_MOUSEBUTTONUP:
                Events.push_back
                        (createMouseButtonEvent(Event::CURSORUP, sdlEvent));
                break;
            case SDL_JOYAXISMOTION:
//                Events.push_back(createAxisEvent(sdlEvent));
                break;
            case SDL_JOYBUTTONDOWN:
//                Events.push_back(createButtonEvent(Event::BUTTON_DOWN, sdlEvent));
                break;
            case SDL_JOYBUTTONUP:
//                Events.push_back(createButtonEvent(Event::BUTTON_UP, sdlEvent));
                break;
            case SDL_KEYDOWN:
                Events.push_back(createKeyEvent(Event::KEYDOWN, sdlEvent));
                break;
            case SDL_KEYUP:
                Events.push_back(createKeyEvent(Event::KEYUP, sdlEvent));
                break;
            case SDL_QUIT:
                {
                    EventPtr pEvent(new Event(Event::QUIT, Event::NONE));
                    Events.push_back(pEvent);
                }
                break;
            case SDL_VIDEORESIZE:
                break;
            default:
                // Ignore unknown events.
                break;
        }
    }
    return Events;
}

EventPtr SDLDisplayEngine::createMouseEvent
        (Event::Type Type, const SDL_Event & SDLEvent, long Button)
{
    int x,y;
    Uint8 buttonState = SDL_GetMouseState(&x, &y);
    x = int((x*m_Width)/m_WindowWidth);
    y = int((y*m_Height)/m_WindowHeight);
    DPoint speed;
    if (m_LastMousePos.x == -1) {
        speed = DPoint(0,0);
    } else {
        double lastFrameTime = 1000/getFramerate();
        speed = DPoint(x-m_LastMousePos.x, y-m_LastMousePos.y)/lastFrameTime;
    }
    MouseEventPtr pEvent(new MouseEvent(Type, (buttonState & SDL_BUTTON(1)) != 0,
            (buttonState & SDL_BUTTON(2)) != 0, (buttonState & SDL_BUTTON(3)) != 0,
            IntPoint(x, y), Button, speed));
    m_LastMousePos = IntPoint(x,y);
    return pEvent; 

}

EventPtr SDLDisplayEngine::createMouseButtonEvent
        (Event::Type Type, const SDL_Event & SDLEvent) 
{
    long Button = 0;
    switch (SDLEvent.button.button) {
        case SDL_BUTTON_LEFT:
            Button = MouseEvent::LEFT_BUTTON;
            break;
        case SDL_BUTTON_MIDDLE:
            Button = MouseEvent::MIDDLE_BUTTON;
            break;
        case SDL_BUTTON_RIGHT:
            Button = MouseEvent::RIGHT_BUTTON;
            break;
        case SDL_BUTTON_WHEELUP:
            Button = MouseEvent::WHEELUP_BUTTON;
            break;
        case SDL_BUTTON_WHEELDOWN:
            Button = MouseEvent::WHEELDOWN_BUTTON;
            break;
    }
    return createMouseEvent(Type, SDLEvent, Button);
 
}

/*
EventPtr SDLDisplayEngine::createAxisEvent(const SDL_Event & SDLEvent)
{
    return new AxisEvent(SDLEvent.jaxis.which, SDLEvent.jaxis.axis,
                SDLEvent.jaxis.value);
}


EventPtr SDLDisplayEngine::createButtonEvent
        (Event::Type Type, const SDL_Event & SDLEvent) 
{
    return new ButtonEvent(Type, SDLEvent.jbutton.which,
                SDLEvent.jbutton.button));
}
*/

EventPtr SDLDisplayEngine::createKeyEvent
        (Event::Type Type, const SDL_Event & SDLEvent)
{
    long KeyCode = KeyCodeTranslationTable[SDLEvent.key.keysym.sym];
    unsigned int Modifiers = key::KEYMOD_NONE;

    if (SDLEvent.key.keysym.mod & KMOD_LSHIFT) 
        { Modifiers |= key::KEYMOD_LSHIFT; }
    if (SDLEvent.key.keysym.mod & KMOD_RSHIFT) 
        { Modifiers |= key::KEYMOD_RSHIFT; }
    if (SDLEvent.key.keysym.mod & KMOD_LCTRL) 
        { Modifiers |= key::KEYMOD_LCTRL; }
    if (SDLEvent.key.keysym.mod & KMOD_RCTRL) 
        { Modifiers |= key::KEYMOD_RCTRL; }
    if (SDLEvent.key.keysym.mod & KMOD_LALT) 
        { Modifiers |= key::KEYMOD_LALT; }
    if (SDLEvent.key.keysym.mod & KMOD_RALT) 
        { Modifiers |= key::KEYMOD_RALT; }
    if (SDLEvent.key.keysym.mod & KMOD_LMETA) 
        { Modifiers |= key::KEYMOD_LMETA; }
    if (SDLEvent.key.keysym.mod & KMOD_RMETA) 
        { Modifiers |= key::KEYMOD_RMETA; }
    if (SDLEvent.key.keysym.mod & KMOD_NUM) 
        { Modifiers |= key::KEYMOD_NUM; }
    if (SDLEvent.key.keysym.mod & KMOD_CAPS) 
        { Modifiers |= key::KEYMOD_CAPS; }
    if (SDLEvent.key.keysym.mod & KMOD_MODE) 
        { Modifiers |= key::KEYMOD_MODE; }
    if (SDLEvent.key.keysym.mod & KMOD_RESERVED) 
        { Modifiers |= key::KEYMOD_RESERVED; }

    KeyEventPtr pEvent(new KeyEvent(Type,
            SDLEvent.key.keysym.scancode, KeyCode,
            SDL_GetKeyName(SDLEvent.key.keysym.sym), SDLEvent.key.keysym.unicode, Modifiers));
    return pEvent;
}

void SDLDisplayEngine::initJoysticks() 
{
/*
    SDL_InitSubSystem(SDL_INIT_JOYSTICK);
    SDL_JoystickEventState(SDL_ENABLE);
    cerr << "**** Number of joysticks: " << SDL_NumJoysticks() << endl;
    for (int i=0; i<SDL_NumJoysticks(); i++) {
        SDL_Joystick * pJoystick = SDL_JoystickOpen(i);
        if (!pJoystick) {
            cerr << "Warning: could not open joystick # " << i << "of"
                << SDL_NumJoysticks() << endl;
        } else {
            printf("Opened Joystick %i\n", i);
            printf("Name: %s\n", SDL_JoystickName(i));
            printf("Number of Axes: %d\n", SDL_JoystickNumAxes(pJoystick));
            printf("Number of Buttons: %d\n", SDL_JoystickNumButtons(pJoystick));
            printf("Number of Balls: %d\n", SDL_JoystickNumBalls(pJoystick));
        }
    }
*/
}

void SDLDisplayEngine::safeSetAttribute( SDL_GLattr attr, int value) {
    int err = SDL_GL_SetAttribute(attr, value);
    if (err == -1) {
        throw Exception(AVG_ERR_VIDEO_GENERAL, SDL_GetError());
    }
}

void SDLDisplayEngine::initTranslationTable()
{
#define TRANSLATION_ENTRY(x) KeyCodeTranslationTable[SDLK_##x] = key::KEY_##x;

    TRANSLATION_ENTRY(UNKNOWN);
    TRANSLATION_ENTRY(BACKSPACE);
    TRANSLATION_ENTRY(TAB);
    TRANSLATION_ENTRY(CLEAR);
    TRANSLATION_ENTRY(RETURN);
    TRANSLATION_ENTRY(PAUSE);
    TRANSLATION_ENTRY(ESCAPE);
    TRANSLATION_ENTRY(SPACE);
    TRANSLATION_ENTRY(EXCLAIM);
    TRANSLATION_ENTRY(QUOTEDBL);
    TRANSLATION_ENTRY(HASH);
    TRANSLATION_ENTRY(DOLLAR);
    TRANSLATION_ENTRY(AMPERSAND);
    TRANSLATION_ENTRY(QUOTE);
    TRANSLATION_ENTRY(LEFTPAREN);
    TRANSLATION_ENTRY(RIGHTPAREN);
    TRANSLATION_ENTRY(ASTERISK);
    TRANSLATION_ENTRY(PLUS);
    TRANSLATION_ENTRY(COMMA);
    TRANSLATION_ENTRY(MINUS);
    TRANSLATION_ENTRY(PERIOD);
    TRANSLATION_ENTRY(SLASH);
    TRANSLATION_ENTRY(0);
    TRANSLATION_ENTRY(1);
    TRANSLATION_ENTRY(2);
    TRANSLATION_ENTRY(3);
    TRANSLATION_ENTRY(4);
    TRANSLATION_ENTRY(5);
    TRANSLATION_ENTRY(6);
    TRANSLATION_ENTRY(7);
    TRANSLATION_ENTRY(8);
    TRANSLATION_ENTRY(9);
    TRANSLATION_ENTRY(COLON);
    TRANSLATION_ENTRY(SEMICOLON);
    TRANSLATION_ENTRY(LESS);
    TRANSLATION_ENTRY(EQUALS);
    TRANSLATION_ENTRY(GREATER);
    TRANSLATION_ENTRY(QUESTION);
    TRANSLATION_ENTRY(AT);
    TRANSLATION_ENTRY(LEFTBRACKET);
    TRANSLATION_ENTRY(BACKSLASH);
    TRANSLATION_ENTRY(RIGHTBRACKET);
    TRANSLATION_ENTRY(CARET);
    TRANSLATION_ENTRY(UNDERSCORE);
    TRANSLATION_ENTRY(BACKQUOTE);
    TRANSLATION_ENTRY(a);
    TRANSLATION_ENTRY(b);
    TRANSLATION_ENTRY(c);
    TRANSLATION_ENTRY(d);
    TRANSLATION_ENTRY(e);
    TRANSLATION_ENTRY(f);
    TRANSLATION_ENTRY(g);
    TRANSLATION_ENTRY(h);
    TRANSLATION_ENTRY(i);
    TRANSLATION_ENTRY(j);
    TRANSLATION_ENTRY(k);
    TRANSLATION_ENTRY(l);
    TRANSLATION_ENTRY(m);
    TRANSLATION_ENTRY(n);
    TRANSLATION_ENTRY(o);
    TRANSLATION_ENTRY(p);
    TRANSLATION_ENTRY(q);
    TRANSLATION_ENTRY(r);
    TRANSLATION_ENTRY(s);
    TRANSLATION_ENTRY(t);
    TRANSLATION_ENTRY(u);
    TRANSLATION_ENTRY(v);
    TRANSLATION_ENTRY(w);
    TRANSLATION_ENTRY(x);
    TRANSLATION_ENTRY(y);
    TRANSLATION_ENTRY(z);
    TRANSLATION_ENTRY(DELETE);
    TRANSLATION_ENTRY(WORLD_0);
    TRANSLATION_ENTRY(WORLD_1);
    TRANSLATION_ENTRY(WORLD_2);
    TRANSLATION_ENTRY(WORLD_3);
    TRANSLATION_ENTRY(WORLD_4);
    TRANSLATION_ENTRY(WORLD_5);
    TRANSLATION_ENTRY(WORLD_6);
    TRANSLATION_ENTRY(WORLD_7);
    TRANSLATION_ENTRY(WORLD_8);
    TRANSLATION_ENTRY(WORLD_9);
    TRANSLATION_ENTRY(WORLD_10);
    TRANSLATION_ENTRY(WORLD_11);
    TRANSLATION_ENTRY(WORLD_12);
    TRANSLATION_ENTRY(WORLD_13);
    TRANSLATION_ENTRY(WORLD_14);
    TRANSLATION_ENTRY(WORLD_15);
    TRANSLATION_ENTRY(WORLD_16);
    TRANSLATION_ENTRY(WORLD_17);
    TRANSLATION_ENTRY(WORLD_18);
    TRANSLATION_ENTRY(WORLD_19);
    TRANSLATION_ENTRY(WORLD_20);
    TRANSLATION_ENTRY(WORLD_21);
    TRANSLATION_ENTRY(WORLD_22);
    TRANSLATION_ENTRY(WORLD_23);
    TRANSLATION_ENTRY(WORLD_24);
    TRANSLATION_ENTRY(WORLD_25);
    TRANSLATION_ENTRY(WORLD_26);
    TRANSLATION_ENTRY(WORLD_27);
    TRANSLATION_ENTRY(WORLD_28);
    TRANSLATION_ENTRY(WORLD_29);
    TRANSLATION_ENTRY(WORLD_30);
    TRANSLATION_ENTRY(WORLD_31);
    TRANSLATION_ENTRY(WORLD_32);
    TRANSLATION_ENTRY(WORLD_33);
    TRANSLATION_ENTRY(WORLD_34);
    TRANSLATION_ENTRY(WORLD_35);
    TRANSLATION_ENTRY(WORLD_36);
    TRANSLATION_ENTRY(WORLD_37);
    TRANSLATION_ENTRY(WORLD_38);
    TRANSLATION_ENTRY(WORLD_39);
    TRANSLATION_ENTRY(WORLD_40);
    TRANSLATION_ENTRY(WORLD_41);
    TRANSLATION_ENTRY(WORLD_42);
    TRANSLATION_ENTRY(WORLD_43);
    TRANSLATION_ENTRY(WORLD_44);
    TRANSLATION_ENTRY(WORLD_45);
    TRANSLATION_ENTRY(WORLD_46);
    TRANSLATION_ENTRY(WORLD_47);
    TRANSLATION_ENTRY(WORLD_48);
    TRANSLATION_ENTRY(WORLD_49);
    TRANSLATION_ENTRY(WORLD_50);
    TRANSLATION_ENTRY(WORLD_51);
    TRANSLATION_ENTRY(WORLD_52);
    TRANSLATION_ENTRY(WORLD_53);
    TRANSLATION_ENTRY(WORLD_54);
    TRANSLATION_ENTRY(WORLD_55);
    TRANSLATION_ENTRY(WORLD_56);
    TRANSLATION_ENTRY(WORLD_57);
    TRANSLATION_ENTRY(WORLD_58);
    TRANSLATION_ENTRY(WORLD_59);
    TRANSLATION_ENTRY(WORLD_60);
    TRANSLATION_ENTRY(WORLD_61);
    TRANSLATION_ENTRY(WORLD_62);
    TRANSLATION_ENTRY(WORLD_63);
    TRANSLATION_ENTRY(WORLD_64);
    TRANSLATION_ENTRY(WORLD_65);
    TRANSLATION_ENTRY(WORLD_66);
    TRANSLATION_ENTRY(WORLD_67);
    TRANSLATION_ENTRY(WORLD_68);
    TRANSLATION_ENTRY(WORLD_69);
    TRANSLATION_ENTRY(WORLD_70);
    TRANSLATION_ENTRY(WORLD_71);
    TRANSLATION_ENTRY(WORLD_72);
    TRANSLATION_ENTRY(WORLD_73);
    TRANSLATION_ENTRY(WORLD_74);
    TRANSLATION_ENTRY(WORLD_75);
    TRANSLATION_ENTRY(WORLD_76);
    TRANSLATION_ENTRY(WORLD_77);
    TRANSLATION_ENTRY(WORLD_78);
    TRANSLATION_ENTRY(WORLD_79);
    TRANSLATION_ENTRY(WORLD_80);
    TRANSLATION_ENTRY(WORLD_81);
    TRANSLATION_ENTRY(WORLD_82);
    TRANSLATION_ENTRY(WORLD_83);
    TRANSLATION_ENTRY(WORLD_84);
    TRANSLATION_ENTRY(WORLD_85);
    TRANSLATION_ENTRY(WORLD_86);
    TRANSLATION_ENTRY(WORLD_87);
    TRANSLATION_ENTRY(WORLD_88);
    TRANSLATION_ENTRY(WORLD_89);
    TRANSLATION_ENTRY(WORLD_90);
    TRANSLATION_ENTRY(WORLD_91);
    TRANSLATION_ENTRY(WORLD_92);
    TRANSLATION_ENTRY(WORLD_93);
    TRANSLATION_ENTRY(WORLD_94);
    TRANSLATION_ENTRY(WORLD_95);
    TRANSLATION_ENTRY(KP0);
    TRANSLATION_ENTRY(KP1);
    TRANSLATION_ENTRY(KP2);
    TRANSLATION_ENTRY(KP3);
    TRANSLATION_ENTRY(KP4);
    TRANSLATION_ENTRY(KP5);
    TRANSLATION_ENTRY(KP6);
    TRANSLATION_ENTRY(KP7);
    TRANSLATION_ENTRY(KP8);
    TRANSLATION_ENTRY(KP9);
    TRANSLATION_ENTRY(KP_PERIOD);
    TRANSLATION_ENTRY(KP_DIVIDE);
    TRANSLATION_ENTRY(KP_MULTIPLY);
    TRANSLATION_ENTRY(KP_MINUS);
    TRANSLATION_ENTRY(KP_PLUS);
    TRANSLATION_ENTRY(KP_ENTER);
    TRANSLATION_ENTRY(KP_EQUALS);
    TRANSLATION_ENTRY(UP);
    TRANSLATION_ENTRY(DOWN);
    TRANSLATION_ENTRY(RIGHT);
    TRANSLATION_ENTRY(LEFT);
    TRANSLATION_ENTRY(INSERT);
    TRANSLATION_ENTRY(HOME);
    TRANSLATION_ENTRY(END);
    TRANSLATION_ENTRY(PAGEUP);
    TRANSLATION_ENTRY(PAGEDOWN);
    TRANSLATION_ENTRY(F1);
    TRANSLATION_ENTRY(F2);
    TRANSLATION_ENTRY(F3);
    TRANSLATION_ENTRY(F4);
    TRANSLATION_ENTRY(F5);
    TRANSLATION_ENTRY(F6);
    TRANSLATION_ENTRY(F7);
    TRANSLATION_ENTRY(F8);
    TRANSLATION_ENTRY(F9);
    TRANSLATION_ENTRY(F10);
    TRANSLATION_ENTRY(F11);
    TRANSLATION_ENTRY(F12);
    TRANSLATION_ENTRY(F13);
    TRANSLATION_ENTRY(F14);
    TRANSLATION_ENTRY(F15);
    TRANSLATION_ENTRY(NUMLOCK);
    TRANSLATION_ENTRY(CAPSLOCK);
    TRANSLATION_ENTRY(SCROLLOCK);
    TRANSLATION_ENTRY(RSHIFT);
    TRANSLATION_ENTRY(LSHIFT);
    TRANSLATION_ENTRY(RCTRL);
    TRANSLATION_ENTRY(LCTRL);
    TRANSLATION_ENTRY(RALT);
    TRANSLATION_ENTRY(LALT);
    TRANSLATION_ENTRY(RMETA);
    TRANSLATION_ENTRY(LMETA);
    TRANSLATION_ENTRY(LSUPER);
    TRANSLATION_ENTRY(RSUPER);
    TRANSLATION_ENTRY(MODE);
    TRANSLATION_ENTRY(COMPOSE);
    TRANSLATION_ENTRY(HELP);
    TRANSLATION_ENTRY(PRINT);
    TRANSLATION_ENTRY(SYSREQ);
    TRANSLATION_ENTRY(BREAK);
    TRANSLATION_ENTRY(MENU);
    TRANSLATION_ENTRY(POWER);
    TRANSLATION_ENTRY(EURO);
    TRANSLATION_ENTRY(UNDO);
}

void SDLDisplayEngine::initTextureMode()
{
    if (m_bShouldUsePOW2Textures) {
        m_bUsePOTTextures = true;
    } else {
        m_bUsePOTTextures = !queryOGLExtension("GL_ARB_texture_non_power_of_two");
    }
}

bool SDLDisplayEngine::usePOTTextures()
{
    return m_bUsePOTTextures;
}

int SDLDisplayEngine::getMaxTexSize() 
{
    if (m_MaxTexSize == 0) {
        glGetIntegerv(GL_MAX_TEXTURE_SIZE, &m_MaxTexSize);
    }
    return m_MaxTexSize;
}

void SDLDisplayEngine::enableTexture(bool bEnable)
{
    if (bEnable != m_bEnableTexture) {
        if (bEnable) {
            glEnable(GL_TEXTURE_2D);
        } else {
            glDisable(GL_TEXTURE_2D);
        }
        m_bEnableTexture = bEnable;
    }
}

void SDLDisplayEngine::enableGLColorArray(bool bEnable)
{
    if (bEnable != m_bEnableGLColorArray) {
        if (bEnable) {
            glEnableClientState(GL_COLOR_ARRAY);
        } else {
            glDisableClientState(GL_COLOR_ARRAY);
        }
        m_bEnableGLColorArray = bEnable;
    }
}

void checkBlendModeError(const char *mode) 
{    
    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        static bool bErrorReported = false;
        if (!bErrorReported) {
            AVG_TRACE(Logger::WARNING, "Blendmode "<< mode <<
                    " not supported by OpenGL implementation.");
            bErrorReported = true;
        }
    }
}

void SDLDisplayEngine::setBlendMode(BlendMode mode)
{
    if (mode != m_BlendMode) {
        switch (mode) {
            case BLEND_BLEND:
                glproc::BlendEquation(GL_FUNC_ADD);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                checkBlendModeError("blend");
                break;
            case BLEND_ADD:
                glproc::BlendEquation(GL_FUNC_ADD);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE);
                checkBlendModeError("add");
                break;
            case BLEND_MIN:
                glproc::BlendEquation(GL_MIN);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                checkBlendModeError("min");
                break;
            case BLEND_MAX:
                glproc::BlendEquation(GL_MAX);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                checkBlendModeError("max");
                break;
        }

        m_BlendMode = mode;
    }
}

// This is what OpenGL calls InternalFormat
int SDLDisplayEngine::getOGLDestMode(PixelFormat pf)
{
    switch (pf) {
        case I8:
            return GL_ALPHA;
        case R8G8B8:
        case B8G8R8:
            return GL_RGB;
        case R8G8B8A8:
        case B8G8R8A8:
            return GL_RGBA;
        case R8G8B8X8:
        case B8G8R8X8:
            return GL_RGBA;    
        default:
            AVG_TRACE(Logger::ERROR, "Unsupported pixel format " << 
                    Bitmap::getPixelFormatString(pf) <<
                    " in SDLDisplayEngine::getOGLDestMode()");
    }
    return 0;
}    

int SDLDisplayEngine::getOGLSrcMode(PixelFormat pf)
{
    switch (pf) {
        case I8:
            return GL_ALPHA;
        case R8G8B8:
            return GL_RGB;
        case B8G8R8:
            return GL_BGR;
        case B8G8R8X8:
        case B8G8R8A8:
            return GL_BGRA;
        case R8G8B8X8:
        case R8G8B8A8:
            return GL_RGBA;
        default:
            AVG_TRACE(Logger::ERROR, "Unsupported pixel format " << 
                    Bitmap::getPixelFormatString(pf) <<
                    " in SDLDisplayEngine::getOGLSrcMode()");
    }
    return 0;
}

int SDLDisplayEngine::getOGLPixelType(PixelFormat pf)
{
    switch (pf) {
        case B8G8R8X8:
        case B8G8R8A8:
//            return GL_UNSIGNED_INT_8_8_8_8_REV;
        default:
            return GL_UNSIGNED_BYTE;
    }
}

OGLMemoryMode SDLDisplayEngine::getMemoryModeSupported()
{
    if (!m_bCheckedMemoryMode) {
        if ((queryOGLExtension("GL_ARB_pixel_buffer_object") || 
             queryOGLExtension("GL_EXT_pixel_buffer_object")) &&
            m_bShouldUsePixelBuffers &&
            !isParallels())
        {
            m_MemoryMode = PBO;
        } else {
            m_MemoryMode = OGL;
        }
        m_bCheckedMemoryMode = true;
    }
    return m_MemoryMode;
}

bool SDLDisplayEngine::isParallels()
{
    static bool bIsParallels = 
            (string((char*)glGetString(GL_VENDOR)).find("Parallels") != string::npos);
    return bIsParallels;
}

void SDLDisplayEngine::setOGLOptions(bool bUsePOW2Textures, bool bUseYCbCrShaders,
        bool bUsePixelBuffers, int MultiSampleSamples, 
        VSyncMode DesiredVSyncMode)
{
    if (m_pScreen) {
        AVG_TRACE(Logger::ERROR, 
                "setOGLOptions called after display initialization. Ignored.");
        return;
    }
    m_bShouldUsePOW2Textures = bUsePOW2Textures;
    m_bShouldUseYCbCrShaders = bUseYCbCrShaders;
    m_bShouldUsePixelBuffers = bUsePixelBuffers;
    m_MultiSampleSamples = MultiSampleSamples;
    m_DesiredVSyncMode = DesiredVSyncMode;
}

}

