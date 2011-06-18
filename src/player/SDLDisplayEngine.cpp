
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
#include "../avgconfigwrapper.h"

#ifdef __APPLE__
#include "SDLMain.h"
#endif

#include "VisibleNode.h"
#include "AVGNode.h"
#include "Shape.h"

#include "Event.h"
#include "MouseEvent.h"
#include "KeyEvent.h"
#ifdef HAVE_XI2_1
#include "XInput21MTInputDevice.h"
#endif
#include "../base/MathHelper.h"
#include "../base/Exception.h"
#include "../base/Logger.h"
#include "../base/ScopeTimer.h"
#include "../base/OSHelper.h"
#include "../base/StringHelper.h"

#include "../graphics/Filterflip.h"
#include "../graphics/Filterfliprgb.h"
#include "../graphics/ShaderRegistry.h"

#include "OGLSurface.h"
#include "OffscreenCanvas.h"

#include <SDL/SDL.h>

#ifdef __APPLE__
#include <ApplicationServices/ApplicationServices.h>
#endif
#ifdef linux
#include <SDL/SDL_syswm.h>
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
#include <math.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

using namespace std;

namespace avg {

double SDLDisplayEngine::s_RefreshRate = 0.0;

void safeSetAttribute(SDL_GLattr attr, int value) 
{
    int err = SDL_GL_SetAttribute(attr, value);
    if (err == -1) {
        throw Exception(AVG_ERR_VIDEO_GENERAL, SDL_GetError());
    }
}

SDLDisplayEngine::SDLDisplayEngine()
    : IInputDevice(EXTRACT_INPUTDEVICE_CLASSNAME(SDLDisplayEngine)),
      m_WindowSize(0,0),
      m_DPI(0,0),
      m_pScreen(0),
      m_VBMethod(VB_NONE),
      m_VBMod(0),
      m_bMouseOverApp(true),
      m_pLastMouseEvent(new MouseEvent(Event::CURSORMOTION, false, false, false, 
            IntPoint(-1, -1), MouseEvent::NO_BUTTON, DPoint(-1, -1), 0)),
      m_NumMouseButtonsDown(0),
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
    m_Gamma[0] = 1.0;
    m_Gamma[1] = 1.0;
    m_Gamma[2] = 1.0;
    initTranslationTable();
}

SDLDisplayEngine::~SDLDisplayEngine()
{
#ifndef _WIN32
    SDL_QuitSubSystem(SDL_INIT_VIDEO);
#endif
}

void SDLDisplayEngine::init(const DisplayParams& dp) 
{
    calcScreenDimensions();
    stringstream ss;
    if (dp.m_Pos.x != -1) {
        ss << dp.m_Pos.x << "," << dp.m_Pos.y;
        setEnv("SDL_VIDEO_WINDOW_POS", ss.str().c_str());
    }
#ifdef linux
    IntPoint oldWindowSize = m_WindowSize;
#endif
    double aspectRatio = double(dp.m_Size.x)/double(dp.m_Size.y);
    if (dp.m_WindowSize == IntPoint(0, 0)) {
        m_WindowSize = dp.m_Size;
    } else if (dp.m_WindowSize.x == 0) {
        m_WindowSize.x = int(dp.m_WindowSize.y*aspectRatio);
        m_WindowSize.y = dp.m_WindowSize.y;
    } else {
        m_WindowSize.x = dp.m_WindowSize.x;
        m_WindowSize.y = int(dp.m_WindowSize.x/aspectRatio);
    }

    switch (dp.m_BPP) {
        case 32:
            safeSetAttribute(SDL_GL_RED_SIZE, 8);
            safeSetAttribute(SDL_GL_GREEN_SIZE, 8);
            safeSetAttribute(SDL_GL_BLUE_SIZE, 8);
            safeSetAttribute(SDL_GL_BUFFER_SIZE, 32);
            break;
        case 24:
            safeSetAttribute(SDL_GL_RED_SIZE, 8);
            safeSetAttribute(SDL_GL_GREEN_SIZE, 8);
            safeSetAttribute(SDL_GL_BLUE_SIZE, 8);
            safeSetAttribute(SDL_GL_BUFFER_SIZE, 24);
            break;
        case 16:
            safeSetAttribute(SDL_GL_RED_SIZE, 5);
            safeSetAttribute(SDL_GL_GREEN_SIZE, 6);
            safeSetAttribute(SDL_GL_BLUE_SIZE, 5);
            safeSetAttribute(SDL_GL_BUFFER_SIZE, 16);
            break;
        case 15:
            safeSetAttribute(SDL_GL_RED_SIZE, 5);
            safeSetAttribute(SDL_GL_GREEN_SIZE, 5);
            safeSetAttribute(SDL_GL_BLUE_SIZE, 5);
            safeSetAttribute(SDL_GL_BUFFER_SIZE, 15);
            break;
        default:
            AVG_TRACE(Logger::ERROR, "Unsupported bpp " << dp.m_BPP <<
                    "in SDLDisplayEngine::init()");
            exit(-1);
    }
    safeSetAttribute(SDL_GL_DEPTH_SIZE, 24);
    safeSetAttribute(SDL_GL_STENCIL_SIZE, 8);
    safeSetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    unsigned int Flags = SDL_OPENGL;
    if (dp.m_bFullscreen) {
        Flags |= SDL_FULLSCREEN;
    }
    m_bIsFullscreen = dp.m_bFullscreen;

    if (!dp.m_bHasWindowFrame) {
        Flags |= SDL_NOFRAME;
    }

    bool bAllMultisampleValuesTested = false;
    m_pScreen = 0;
    while (!bAllMultisampleValuesTested && !m_pScreen) {
        if (m_GLConfig.m_MultiSampleSamples > 1) {
            safeSetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
            safeSetAttribute(SDL_GL_MULTISAMPLESAMPLES, m_GLConfig.m_MultiSampleSamples);
        } else {
            safeSetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);
            safeSetAttribute(SDL_GL_MULTISAMPLESAMPLES, 0);
        }
        m_pScreen = SDL_SetVideoMode(m_WindowSize.x, m_WindowSize.y, dp.m_BPP, Flags);
        if (!m_pScreen) {
            switch (m_GLConfig.m_MultiSampleSamples) {
                case 1:
                    bAllMultisampleValuesTested = true;
                    break;
                case 2:  
                    m_GLConfig.m_MultiSampleSamples = 1;
                    break;
                case 4:  
                    m_GLConfig.m_MultiSampleSamples = 2;
                    break;
                case 8:  
                    m_GLConfig.m_MultiSampleSamples = 4;
                    break;
                default:
                    m_GLConfig.m_MultiSampleSamples = 8;
                    break;
            }
        }
    }
#ifdef HAVE_XI2_1
    SDL_EventState(SDL_SYSWMEVENT, SDL_ENABLE);
    m_pXIMTInputDevice = 0;
#endif
    if (!m_pScreen) {
        throw Exception(AVG_ERR_UNSUPPORTED, string("Setting SDL video mode failed: ")
                + SDL_GetError() + ". (size=" + toString(m_WindowSize) + ", bpp=" + 
                toString(dp.m_BPP) + ", multisamplesamples=" + 
                toString(m_GLConfig.m_MultiSampleSamples) + ").");
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
        m_GLConfig.m_MultiSampleSamples = 1;
    } else {
        if (m_GLConfig.m_MultiSampleSamples > 1) {
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

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    setGamma(dp.m_Gamma[0], dp.m_Gamma[1], dp.m_Gamma[2]);
    showCursor(dp.m_bShowCursor);
    if (dp.m_Framerate == 0) {
        setVBlankRate(dp.m_VBRate);
    } else {
        setFramerate(dp.m_Framerate);
    }

    checkShaderSupport();
    m_BlendMode = BLEND_ADD;
    setBlendMode(BLEND_BLEND, false);

    m_Size = dp.m_Size;
    // SDL sets up a signal handler we really don't want.
    signal(SIGSEGV, SIG_DFL);
    logConfig();


    SDL_EnableUNICODE(1);
}

#ifdef _WIN32
#pragma warning(disable: 4996)
#endif
void SDLDisplayEngine::teardown()
{
    if (m_pScreen) {
        if (m_Gamma[0] != 1.0 || m_Gamma[1] != 1.0 || m_Gamma[2] != 1.0) {
            SDL_SetGamma(1.0, 1.0, 1.0);
        }
#ifdef linux
        // Workaround for broken mouse cursor on exit under Ubuntu 8.04.
        SDL_ShowCursor(SDL_ENABLE);
//        SDL_SetVideoMode(m_WindowWidth, m_WindowHeight, 24, 0);
#endif
        m_pScreen = 0;
    }
    VertexArray::deleteBufferCache();
    GPUFilter::glContextGone();
    FBO::deleteCache();
    ShaderRegistry::kill();
}

double SDLDisplayEngine::getRefreshRate() 
{
    if (s_RefreshRate == 0.0) {
        calcRefreshRate();
    }
    return s_RefreshRate;
}

void SDLDisplayEngine::setGamma(double red, double green, double blue)
{
    if (red > 0) {
        AVG_TRACE(Logger::CONFIG, "Setting gamma to " << red << ", " << green << ", " << blue);
        int err = SDL_SetGamma(float(red), float(green), float(blue));
        m_Gamma[0] = red;
        m_Gamma[1] = green;
        m_Gamma[2] = blue;
        if (err == -1) {
            AVG_TRACE(Logger::WARNING, "Unable to set display gamma.");
        }
    }
}

void SDLDisplayEngine::setMousePos(const IntPoint& pos)
{
    SDL_WarpMouse(pos.x, pos.y);
}

int SDLDisplayEngine::getKeyModifierState() const
{
    return SDL_GetModState();
}

void SDLDisplayEngine::logConfig() 
{
    AVG_TRACE(Logger::CONFIG, "OpenGL configuration: ");
    AVG_TRACE(Logger::CONFIG, "  OpenGL version: " << glGetString(GL_VERSION));
    AVG_TRACE(Logger::CONFIG, "  OpenGL vendor: " << glGetString(GL_VENDOR));
    AVG_TRACE(Logger::CONFIG, "  OpenGL renderer: " << glGetString(GL_RENDERER));
    m_GLConfig.log();
    switch (getMemoryModeSupported()) {
        case MM_PBO:
            AVG_TRACE(Logger::CONFIG, "  Using pixel buffer objects.");
            break;
        case MM_OGL:
            AVG_TRACE(Logger::CONFIG, "  Not using GL memory extensions.");
            break;
    }
    AVG_TRACE(Logger::CONFIG, "  Max. texture size is " << getMaxTexSize());
}

void SDLDisplayEngine::calcScreenDimensions()
{
    if (m_DPI == DPoint(0,0)) {
        const SDL_VideoInfo* pInfo = SDL_GetVideoInfo();
        m_ScreenResolution = IntPoint(pInfo->current_w, pInfo->current_h);
#ifdef linux
        Display * pDisplay = XOpenDisplay(0);
        DPoint displayMM(DisplayWidthMM(pDisplay,0), DisplayHeightMM(pDisplay,0));
        DPoint displayInches = displayMM/25.4;
        m_DPI.x = m_ScreenResolution.x/displayInches.x;
        m_DPI.y = m_ScreenResolution.y/displayInches.y;
#elif defined __APPLE__
        CGSize displayMM = CGDisplayScreenSize(CGMainDisplayID());
        DPoint displayInches(DPoint(displayMM.width, displayMM.height)/25.4);
        m_DPI.x = m_ScreenResolution.x/displayInches.x;
        m_DPI.y = m_ScreenResolution.y/displayInches.y;
#elif defined WIN32
#endif
    }
}

static ProfilingZoneID PushClipRectProfilingZone("pushClipRect");

bool SDLDisplayEngine::pushClipRect(const DRect& rc)
{
    ScopeTimer timer(PushClipRectProfilingZone);

    m_ClipRects.push_back(rc);
    clip(true);

    return true;
}

static ProfilingZoneID PopClipRectProfilingZone("popClipRect");

void SDLDisplayEngine::popClipRect()
{
    ScopeTimer timer(PopClipRectProfilingZone);
    
    clip(false);
    m_ClipRects.pop_back();
}

void SDLDisplayEngine::pushTransform(const DPoint& translate, double angle, 
        const DPoint& pivot)
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
    if (!m_ClipRects.empty()) {
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

static ProfilingZoneID SwapBufferProfilingZone("Render - swap buffers");

void SDLDisplayEngine::swapBuffers()
{
    ScopeTimer timer(SwapBufferProfilingZone);
    SDL_GL_SwapBuffers();
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "swapBuffers()");
    AVG_TRACE(Logger::BLTS, "GL SwapBuffers");
}

bool SDLDisplayEngine::isUsingShaders() const
{
    return m_GLConfig.m_bUseShaders;
}

void SDLDisplayEngine::showCursor(bool bShow)
{
#ifdef _WIN32
#define MAX_CORE_POINTERS   6
    // Hack to fix a pointer issue with fullscreen, SDL and touchscreens
    // Refer to Mantis bug #140
    for (int i = 0; i < MAX_CORE_POINTERS; ++i) {
        ShowCursor(bShow);
    }
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
    BitmapPtr pBmp (new Bitmap(m_WindowSize, B8G8R8X8, "screenshot"));
    string sTmp;
    bool bBroken = getEnv("AVG_BROKEN_READBUFFER", sTmp);
    if (bBroken) {
        // Workaround for buggy GL_FRONT on some machines.
        glReadBuffer(GL_BACK);
    } else {
        glReadBuffer(GL_FRONT);
    }
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "SDLDisplayEngine::screenshot:glReadBuffer()");
    glReadPixels(0, 0, m_WindowSize.x, m_WindowSize.y, GL_BGRA, GL_UNSIGNED_BYTE, 
            pBmp->getPixels());
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "SDLDisplayEngine::screenshot:glReadPixels()");
    FilterFlip().applyInPlace(pBmp);
    return pBmp;
}

IntPoint SDLDisplayEngine::getSize()
{
    return m_Size;
}

void SDLDisplayEngine::checkShaderSupport()
{
    m_GLConfig.m_bUseShaders = (queryOGLExtension("GL_ARB_fragment_shader") && 
            getMemoryModeSupported() == MM_PBO &&
            !m_GLConfig.m_bUsePOTTextures && m_GLConfig.m_bUseShaders);
    if (m_GLConfig.m_bUseShaders) {
        OGLSurface::createShader();
    }
}

void SDLDisplayEngine::initMacVBlank(int rate)
{
#ifdef __APPLE__
    CGLContextObj context = CGLGetCurrentContext();
    AVG_ASSERT (context);
    if (rate > 1) {
        AVG_TRACE(Logger::WARNING,
                "VBlank rate set to " << rate 
                << " but Mac OS X only supports 1. Assuming 1.");
    }
#if MAC_OS_X_VERSION_10_5
    const GLint l = 1;
#else
    const long l = 1;
#endif
    CGLError err = CGLSetParameter(context, kCGLCPSwapInterval, &l);
    AVG_ASSERT(!err);
#endif
}

bool SDLDisplayEngine::initVBlank(int rate) 
{
    if (rate > 0) {
#ifdef __APPLE__
        initMacVBlank(rate);
        m_VBMethod = VB_APPLE;
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
        if (getenv("__GL_SYNC_TO_VBLANK") != 0) {
            AVG_TRACE(Logger::WARNING, 
                    "__GL_SYNC_TO_VBLANK set. This interferes with libavg vblank handling.");
            m_VBMethod = VB_NONE;
        } else {
            if (queryGLXExtension("GLX_SGI_swap_control")) {
                m_VBMethod = VB_SGI;
                glproc::SwapIntervalSGI(rate);

                m_bFirstVBFrame = true;
            } else {
                AVG_TRACE(Logger::WARNING,
                        "Linux VBlank setup failed: OpenGL Extension not supported.");
                m_VBMethod = VB_NONE;
            }
        }
#endif
    } else {
        switch (m_VBMethod) {
            case VB_APPLE:
                initMacVBlank(0);
                break;
            case VB_WIN:
#ifdef _WIN32
                glproc::SwapIntervalEXT(0);
#endif
                break;
            case VB_SGI:
#ifdef linux            
                if (queryGLXExtension("GLX_SGI_swap_control")) {
                    glproc::SwapIntervalSGI(rate);
                }
#endif
                break;
            default:
                break;
        }
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

bool SDLDisplayEngine::vbWait(int rate)
{
    switch(m_VBMethod) {
        case VB_SGI:
        case VB_APPLE:
        case VB_WIN:
            return true;
        case VB_NONE:
        default:
            AVG_ASSERT(false);
            return false;
    }
}

void SDLDisplayEngine::calcRefreshRate()
{
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
    Display * pDisplay = XOpenDisplay(0);
    int pixelClock;
    XF86VidModeModeLine modeLine;
    bool bOK = XF86VidModeGetModeLine (pDisplay, DefaultScreen(pDisplay), 
            &pixelClock, &modeLine);
    if (!bOK) {
        AVG_TRACE (Logger::WARNING, 
                "Could not get current refresh rate (XF86VidModeGetModeLine failed).");
        AVG_TRACE (Logger::WARNING, 
                "Defaulting to 60 Hz refresh rate.");
    }
    double HSyncRate = pixelClock*1000.0/modeLine.htotal;
    s_RefreshRate = HSyncRate/modeLine.vtotal;
    XCloseDisplay(pDisplay);
#endif
    if (s_RefreshRate == 0 || isnan(s_RefreshRate)) {
        s_RefreshRate = 60;
    }
    if (lastRefreshRate != s_RefreshRate) {
        AVG_TRACE(Logger::CONFIG, "Vertical Refresh Rate: " << s_RefreshRate);
    }

}

vector<long> SDLDisplayEngine::KeyCodeTranslationTable(SDLK_LAST, key::KEY_UNKNOWN);

const char * getEventTypeName(unsigned char type) 
{
    switch (type) {
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
    vector<EventPtr> events;

    while (SDL_PollEvent(&sdlEvent)) {
        EventPtr pNewEvent;
        switch (sdlEvent.type) {
            case SDL_MOUSEMOTION:
                if (m_bMouseOverApp) {
                    pNewEvent = createMouseEvent(Event::CURSORMOTION, sdlEvent, 
                            MouseEvent::NO_BUTTON);
                    CursorEventPtr pNewCursorEvent = 
                            boost::dynamic_pointer_cast<CursorEvent>(pNewEvent);
                    if (!events.empty()) {
                        CursorEventPtr pLastEvent = 
                                boost::dynamic_pointer_cast<CursorEvent>(events.back());
                        if (pLastEvent && *pNewCursorEvent == *pLastEvent) {
                            pNewEvent = EventPtr();
                        }
                    }
                }
                break;
            case SDL_MOUSEBUTTONDOWN:
                pNewEvent = createMouseButtonEvent(Event::CURSORDOWN, sdlEvent);
                break;
            case SDL_MOUSEBUTTONUP:
                pNewEvent = createMouseButtonEvent(Event::CURSORUP, sdlEvent);
                break;
            case SDL_JOYAXISMOTION:
//                pNewEvent = createAxisEvent(sdlEvent));
                break;
            case SDL_JOYBUTTONDOWN:
//                pNewEvent = createButtonEvent(Event::BUTTON_DOWN, sdlEvent));
                break;
            case SDL_JOYBUTTONUP:
//                pNewEvent = createButtonEvent(Event::BUTTON_UP, sdlEvent));
                break;
            case SDL_KEYDOWN:
                pNewEvent = createKeyEvent(Event::KEYDOWN, sdlEvent);
                break;
            case SDL_KEYUP:
                pNewEvent = createKeyEvent(Event::KEYUP, sdlEvent);
                break;
            case SDL_QUIT:
                pNewEvent = EventPtr(new Event(Event::QUIT, Event::NONE));
                break;
            case SDL_VIDEORESIZE:
                break;
            case SDL_SYSWMEVENT:
                {
#ifdef HAVE_XI2_1
                    SDL_SysWMmsg* pMsg = sdlEvent.syswm.msg;
                    AVG_ASSERT(pMsg->subsystem == SDL_SYSWM_X11);
                    if (m_pXIMTInputDevice) {
                        m_pXIMTInputDevice->handleXIEvent(pMsg->event.xevent);
                    }
#endif
                }
                break;
            default:
                // Ignore unknown events.
                break;
        }
        if (pNewEvent) {
            events.push_back(pNewEvent);
        }
    }
    return events;
}

void SDLDisplayEngine::setXIMTInputDevice(XInput21MTInputDevice* pInputDevice)
{
    AVG_ASSERT(!m_pXIMTInputDevice);
    m_pXIMTInputDevice = pInputDevice;
}

EventPtr SDLDisplayEngine::createMouseEvent(Event::Type type, const SDL_Event& sdlEvent,
        long button)
{
    int x, y;
    Uint8 buttonState = SDL_GetMouseState(&x, &y);
    x = int((x*m_Size.x)/m_WindowSize.x);
    y = int((y*m_Size.y)/m_WindowSize.y);
    DPoint lastMousePos = m_pLastMouseEvent->getPos();
    DPoint speed;
    if (lastMousePos.x == -1) {
        speed = DPoint(0,0);
    } else {
        double lastFrameTime = 1000/getEffectiveFramerate();
        speed = DPoint(x-lastMousePos.x, y-lastMousePos.y)/lastFrameTime;
    }
    MouseEventPtr pEvent(new MouseEvent(type, (buttonState & SDL_BUTTON(1)) != 0,
            (buttonState & SDL_BUTTON(2)) != 0, (buttonState & SDL_BUTTON(3)) != 0,
            IntPoint(x, y), button, speed));

    m_pLastMouseEvent = pEvent;
    return pEvent; 
}

EventPtr SDLDisplayEngine::createMouseButtonEvent(Event::Type type, 
        const SDL_Event& sdlEvent) 
{
    long button = 0;
    switch (sdlEvent.button.button) {
        case SDL_BUTTON_LEFT:
            button = MouseEvent::LEFT_BUTTON;
            break;
        case SDL_BUTTON_MIDDLE:
            button = MouseEvent::MIDDLE_BUTTON;
            break;
        case SDL_BUTTON_RIGHT:
            button = MouseEvent::RIGHT_BUTTON;
            break;
        case SDL_BUTTON_WHEELUP:
            button = MouseEvent::WHEELUP_BUTTON;
            break;
        case SDL_BUTTON_WHEELDOWN:
            button = MouseEvent::WHEELDOWN_BUTTON;
            break;
    }
    return createMouseEvent(type, sdlEvent, button);
 
}

/*
EventPtr SDLDisplayEngine::createAxisEvent(const SDL_Event & sdlEvent)
{
    return new AxisEvent(sdlEvent.jaxis.which, sdlEvent.jaxis.axis,
                sdlEvent.jaxis.value);
}


EventPtr SDLDisplayEngine::createButtonEvent
        (Event::Type type, const SDL_Event & sdlEvent) 
{
    return new ButtonEvent(type, sdlEvent.jbutton.which,
                sdlEvent.jbutton.button));
}
*/

EventPtr SDLDisplayEngine::createKeyEvent(Event::Type type, const SDL_Event& sdlEvent)
{
    long keyCode = KeyCodeTranslationTable[sdlEvent.key.keysym.sym];
    unsigned int modifiers = key::KEYMOD_NONE;

    if (sdlEvent.key.keysym.mod & KMOD_LSHIFT) 
        { modifiers |= key::KEYMOD_LSHIFT; }
    if (sdlEvent.key.keysym.mod & KMOD_RSHIFT) 
        { modifiers |= key::KEYMOD_RSHIFT; }
    if (sdlEvent.key.keysym.mod & KMOD_LCTRL) 
        { modifiers |= key::KEYMOD_LCTRL; }
    if (sdlEvent.key.keysym.mod & KMOD_RCTRL) 
        { modifiers |= key::KEYMOD_RCTRL; }
    if (sdlEvent.key.keysym.mod & KMOD_LALT) 
        { modifiers |= key::KEYMOD_LALT; }
    if (sdlEvent.key.keysym.mod & KMOD_RALT) 
        { modifiers |= key::KEYMOD_RALT; }
    if (sdlEvent.key.keysym.mod & KMOD_LMETA) 
        { modifiers |= key::KEYMOD_LMETA; }
    if (sdlEvent.key.keysym.mod & KMOD_RMETA) 
        { modifiers |= key::KEYMOD_RMETA; }
    if (sdlEvent.key.keysym.mod & KMOD_NUM) 
        { modifiers |= key::KEYMOD_NUM; }
    if (sdlEvent.key.keysym.mod & KMOD_CAPS) 
        { modifiers |= key::KEYMOD_CAPS; }
    if (sdlEvent.key.keysym.mod & KMOD_MODE) 
        { modifiers |= key::KEYMOD_MODE; }
    if (sdlEvent.key.keysym.mod & KMOD_RESERVED) 
        { modifiers |= key::KEYMOD_RESERVED; }

    KeyEventPtr pEvent(new KeyEvent(type,
            sdlEvent.key.keysym.scancode, keyCode,
            SDL_GetKeyName(sdlEvent.key.keysym.sym), sdlEvent.key.keysym.unicode, modifiers));
    return pEvent;
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
    if (!m_GLConfig.m_bUsePOTTextures) {
        m_GLConfig.m_bUsePOTTextures = 
                !queryOGLExtension("GL_ARB_texture_non_power_of_two");
    }
}

bool SDLDisplayEngine::usePOTTextures()
{
    return m_GLConfig.m_bUsePOTTextures;
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

void checkBlendModeError(const char * sMode) 
{    
    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        static bool bErrorReported = false;
        if (!bErrorReported) {
            AVG_TRACE(Logger::WARNING, "Blendmode "<< sMode <<
                    " not supported by OpenGL implementation.");
            bErrorReported = true;
        }
    }
}

void SDLDisplayEngine::setBlendMode(BlendMode mode, bool bPremultipliedAlpha)
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
                checkBlendModeError("blend");
                break;
            case BLEND_ADD:
                glproc::BlendEquation(GL_FUNC_ADD);
                glproc::BlendFuncSeparate(srcFunc, GL_ONE, GL_ONE, GL_ONE);
                checkBlendModeError("add");
                break;
            case BLEND_MIN:
                glproc::BlendEquation(GL_MIN);
                glproc::BlendFuncSeparate(srcFunc, GL_ONE_MINUS_SRC_ALPHA, 
                        GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
                checkBlendModeError("min");
                break;
            case BLEND_MAX:
                glproc::BlendEquation(GL_MAX);
                glproc::BlendFuncSeparate(srcFunc, GL_ONE_MINUS_SRC_ALPHA, 
                        GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
                checkBlendModeError("max");
                break;
            case BLEND_COPY:
                glproc::BlendEquation(GL_FUNC_ADD);
                glBlendFunc(GL_ONE, GL_ZERO);
                break;
            default:
                AVG_ASSERT(false);
        }

        m_BlendMode = mode;
        m_bPremultipliedAlpha = bPremultipliedAlpha;
    }
}

OGLMemoryMode SDLDisplayEngine::getMemoryModeSupported()
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

void SDLDisplayEngine::setOGLOptions(const GLConfig& glConfig)
{
    if (m_pScreen) {
        AVG_TRACE(Logger::ERROR, 
                "setOGLOptions called after display initialization. Ignored.");
        return;
    }
    m_GLConfig = glConfig;
}

const GLConfig& SDLDisplayEngine::getOGLOptions() const
{
    return m_GLConfig;
}

const IntPoint& SDLDisplayEngine::getWindowSize() const
{
    return m_WindowSize;
}

bool SDLDisplayEngine::isFullscreen() const
{
    return m_bIsFullscreen;
}

IntPoint SDLDisplayEngine::getScreenResolution()
{
    calcScreenDimensions();
    return m_ScreenResolution;
}

DPoint SDLDisplayEngine::getDPI()
{
    calcScreenDimensions();
    return m_DPI;
}

DPoint SDLDisplayEngine::getPhysicalScreenDimensions()
{
    calcScreenDimensions();
    DPoint size;
    DPoint screenRes = DPoint(getScreenResolution());
    size.x = screenRes.x/getDPI().x*25.4;
    size.y = screenRes.y/getDPI().y*25.4;
    return size;
}

void SDLDisplayEngine::setMainFBO(FBOPtr pFBO)
{
    m_pFBO = pFBO;
}

FBOPtr SDLDisplayEngine::getMainFBO() const
{
    return m_pFBO;
}

}
