//
// $Id$
//

#include "AVGSDLDisplayEngine.h"
#include "AVGException.h"
#include "AVGRegion.h"
#include "AVGPlayer.h"
#include "AVGNode.h"
#include "AVGLogger.h"
#include "AVGFramerateManager.h"
#include "AVGSDLFontManager.h"
#include "AVGEvent.h"
#include "AVGMouseEvent.h"
#include "AVGKeyEvent.h"
#include "AVGWindowEvent.h"
#include "AVGOGLSurface.h"
#include "OGLHelper.h"

#include <paintlib/plbitmap.h>
#include <paintlib/plrect.h>
#include <paintlib/Filter/plfilterflip.h>

#include <GL/gl.h>
#include <GL/glu.h>

#include <signal.h>
#include <iostream>
#include <sstream>
#include <unistd.h>

using namespace std;

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

AVGSDLDisplayEngine::AVGSDLDisplayEngine()
    : m_pScreen(0)
{
    if (SDL_InitSubSystem(SDL_INIT_VIDEO)==-1) {
        AVG_TRACE(IAVGPlayer::DEBUG_ERROR, "Can't init SDL display subsystem.");
        exit(-1);
    }
    initTranslationTable();
}

AVGSDLDisplayEngine::~AVGSDLDisplayEngine()
{
    delete m_pFontManager;
    m_pFontManager = 0;
    if (m_pScreen) {
        teardown();
    }
}

void AVGSDLDisplayEngine::init(int width, int height, bool isFullscreen, 
        int bpp, const string & sFontPath)
{
    switch (bpp) {
        case 32:
            SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 8 );
            SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 8 );
            SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 8 );
            SDL_GL_SetAttribute( SDL_GL_BUFFER_SIZE, 32 );
            break;
        case 24:
            SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 8 );
            SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 8 );
            SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 8 );
            SDL_GL_SetAttribute( SDL_GL_BUFFER_SIZE, 24 );
            break;
        case 16:
            SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 5 );
            SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 6 );
            SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 5 );
            SDL_GL_SetAttribute( SDL_GL_BUFFER_SIZE, 16 );
            break;
        case 15:
            SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 5 );
            SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 5 );
            SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 5 );
            SDL_GL_SetAttribute( SDL_GL_BUFFER_SIZE, 15 );
            break;
        default:
            AVG_TRACE(IAVGPlayer::DEBUG_ERROR, "Unsupported bpp " << bpp <<
                    "in AVGSDLDisplayEngine::init()");
            exit(-1);
    }
    SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 0 );
    SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );

    unsigned int Flags = SDL_OPENGL;
    if (isFullscreen) {
        Flags |= SDL_FULLSCREEN;
    }
    m_pScreen = SDL_SetVideoMode(width, height, bpp, Flags);
    if (!m_pScreen) {
        AVG_TRACE(IAVGPlayer::DEBUG_ERROR, "Setting SDL video mode failed: " 
                << SDL_GetError());
        exit(-1);
    }   
    SDL_WM_SetCaption("AVG Renderer", 0);
//    dumpSDLGLParams();

    glEnable(GL_BLEND);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "init: glEnable(GL_BLEND)");
    glShadeModel(GL_FLAT);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "init: glShadeModel()");
    glDisable(GL_DEPTH_TEST);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "init: glDisable(GL_DEPTH_TEST)");
    int TexMode = AVGOGLSurface::getTextureMode();
    glEnable(TexMode);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "init: glEnable(TexMode);");

    m_Width = width;
    m_Height = height;
    initInput();
    m_pFontManager = new AVGSDLFontManager(sFontPath);
    // SDL sets up a signal handler we really don't want.
//    signal(SIGSEGV, SIG_DFL);
    if (getenv("__GL_SYNC_TO_VBLANK") == 0) {
        AVG_TRACE(IAVGPlayer::DEBUG_WARNING, 
                "__GL_SYNC_TO_VBLANK not set.");
    }
}

void AVGSDLDisplayEngine::teardown()
{
    SDL_QuitSubSystem(SDL_INIT_VIDEO);
}

void AVGSDLDisplayEngine::render(AVGNode * pRootNode, 
        AVGFramerateManager * pFramerateManager, bool bRenderEverything)
{
    pRootNode->prepareRender(0, pRootNode->getAbsViewport());
    glClearColor(0.0, 0.0, 0.0, 0.0); 
    glClear(GL_COLOR_BUFFER_BIT);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            "AVGSDLDisplayEngine::render::glClear()");

    glViewport(0, 0, m_Width, m_Height);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            "AVGSDLDisplayEngine::render: glViewport()");
    glMatrixMode(GL_PROJECTION);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            "AVGSDLDisplayEngine::render: glMatrixMode()");
    glLoadIdentity();
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            "AVGSDLDisplayEngine::render: glLoadIdentity()");
    gluOrtho2D(0, m_Width, m_Height, 0);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            "AVGSDLDisplayEngine::render: gluOrtho2D()");
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE); 
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            "AVGSDLDisplayEngine::render: glTexEnvf()");
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, 
            "AVGSDLDisplayEngine::render: glBlendFunc()");
    
    const AVGDRect rc(0,0, m_Width, m_Height);
    glMatrixMode(GL_MODELVIEW);
    pRootNode->maybeRender(rc);
    pFramerateManager->FrameWait();
    swapBuffers();
    pFramerateManager->CheckJitter();
}
 
void AVGSDLDisplayEngine::setClipRect()
{
    m_ClipRects.clear();
    m_ClipRects.push_back(AVGDRect(0, 0, m_Width, m_Height));
}

bool AVGSDLDisplayEngine::pushClipRect(const AVGDRect& rc, bool bClip)
{
    m_ClipRects.push_back(rc);

    glPushMatrix();
    AVG_TRACE(AVGPlayer::DEBUG_BLTS, "Clip set to " << 
            rc.tl.x << "x" << rc.tl.y << 
            ", width: " << rc.Width() << ", height: " << 
            rc.Height());
    if (bClip) {
        clip();
    }
    return true;
}

void AVGSDLDisplayEngine::popClipRect()
{
    glPopMatrix();
    m_ClipRects.pop_back();
    clip();
}

const AVGDRect& AVGSDLDisplayEngine::getClipRect() 
{
    return m_ClipRects.back();
}

void AVGSDLDisplayEngine::blt32(IAVGSurface * pSurface, const AVGDRect* pDestRect, 
        double opacity, double angle, const AVGDPoint& pivot)
{
    AVGOGLSurface * pOGLSurface = dynamic_cast<AVGOGLSurface*>(pSurface);
    glColor4f(1.0f, 1.0f, 1.0f, opacity);
    pOGLSurface->blt(pDestRect, opacity, angle, pivot);
}

void AVGSDLDisplayEngine::blta8(IAVGSurface * pSurface, 
        const AVGDRect* pDestRect, double opacity, 
        const PLPixel32& color, double angle, 
        const AVGDPoint& pivot)
{
    AVGOGLSurface * pOGLSurface = dynamic_cast<AVGOGLSurface*>(pSurface);
    glColor4f(float(color.GetR())/256, float(color.GetG())/256, 
            float(color.GetB())/256, opacity);
    pOGLSurface->blt(pDestRect, opacity, angle, pivot);
}


void AVGSDLDisplayEngine::clip()
{
    AVGDRect rc = m_ClipRects.back();
    double xEqn[4] = {1.0, 0.0, 0.0, 0.0};
    xEqn[3] = -rc.tl.x;
    glClipPlane(GL_CLIP_PLANE0, xEqn);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "setClipRect: glClipPlane(0)");
    glEnable (GL_CLIP_PLANE0);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "setClipRect: glEnable(0)");

    xEqn[0] = -1;
    xEqn[1] = 0;
    xEqn[2] = 0;
    xEqn[3] = rc.br.x;
    glClipPlane(GL_CLIP_PLANE1, xEqn);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "setClipRect: glClipPlane(1)");
    glEnable (GL_CLIP_PLANE1);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "setClipRect: glEnable(1)");

    double yEqn[4] = {0.0, 1.0, 0.0, 0.0};
    yEqn[3] = -rc.tl.y;
    glClipPlane(GL_CLIP_PLANE2, yEqn);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "setClipRect: glClipPlane(2)");
    glEnable (GL_CLIP_PLANE2);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "setClipRect: glEnable(2)");
    yEqn[0] = 0;
    yEqn[1] = -1.0;
    yEqn[2] = 0;
    yEqn[3] = rc.br.y;
    glClipPlane(GL_CLIP_PLANE3, yEqn);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "setClipRect: glClipPlane(3)");
    glEnable (GL_CLIP_PLANE3);
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "setClipRect: glEnable(3)");
}

void AVGSDLDisplayEngine::setDirtyRect(const AVGDRect& rc) 
{
    m_DirtyRect = rc;
    
    AVG_TRACE(AVGPlayer::DEBUG_BLTS, "Dirty rect: " << m_DirtyRect.tl.x << "x" << 
            m_DirtyRect.tl.y << ", width: " << m_DirtyRect.Width() << 
            ", height: " << m_DirtyRect.Height());
}

void AVGSDLDisplayEngine::swapBuffers()
{
    SDL_GL_SwapBuffers();
    AVG_TRACE(AVGPlayer::DEBUG_BLTS, "GL SwapBuffers");
    OGLErrorCheck(AVG_ERR_VIDEO_GENERAL, "swapBuffers()");
}

IAVGSurface * AVGSDLDisplayEngine::createSurface()
{
    return new AVGOGLSurface;
}

void AVGSDLDisplayEngine::surfaceChanged(IAVGSurface * pSurface) 
{
    AVGOGLSurface * pOGLSurface = dynamic_cast<AVGOGLSurface *>(pSurface);
    pOGLSurface->bind();
}

AVGFontManager * AVGSDLDisplayEngine::getFontManager()
{
    return m_pFontManager;
}

bool AVGSDLDisplayEngine::hasYUVSupport()
{
    return false;
//    return queryOGLExtension("GL_NVX_ycrcb");
}

bool AVGSDLDisplayEngine::supportsBpp(int bpp)
{
    return (bpp == 24 || bpp == 32);
}

bool AVGSDLDisplayEngine::hasRGBOrdering()
{
    return true;
}

void AVGSDLDisplayEngine::showCursor (bool bShow)
{
    if (bShow) {
        SDL_ShowCursor(SDL_ENABLE);
    } else {
        SDL_ShowCursor(SDL_DISABLE);
    }
}

void AVGSDLDisplayEngine::screenshot (const string& sFilename, PLBmp& Bmp)
{
    Bmp.Create(m_Width, m_Height, 32, false, false, 0, 0, PLPoint(72, 72));
    glReadBuffer(GL_FRONT);
    glReadPixels(0, 0, m_Width, m_Height, GL_RGBA, GL_UNSIGNED_BYTE, 
            Bmp.GetLineArray()[0]);
    Bmp.ApplyFilter(PLFilterFlip());
}

int AVGSDLDisplayEngine::getWidth()
{
    return m_Width;
}

int AVGSDLDisplayEngine::getHeight()
{
    return m_Height;
}

int AVGSDLDisplayEngine::getBPP()
{
    return m_bpp;
}

vector<long> AVGSDLDisplayEngine::KeyCodeTranslationTable(SDLK_LAST, AVGKeyEvent::KEY_UNKNOWN);

void AVGSDLDisplayEngine::initInput()
{
    initJoysticks();
}

vector<AVGEvent *> AVGSDLDisplayEngine::pollEvents()
{
    SDL_Event sdlEvent;
    vector<AVGEvent *> Events;

    while(SDL_PollEvent(&sdlEvent)){
        switch(sdlEvent.type){
            case SDL_MOUSEMOTION:
                Events.push_back
                        (createMouseMotionEvent(AVGEvent::MOUSEMOTION, sdlEvent));
                break;
            case SDL_MOUSEBUTTONDOWN:
                Events.push_back
                        (createMouseButtonEvent(AVGEvent::MOUSEBUTTONDOWN, sdlEvent));
                break;
            case SDL_MOUSEBUTTONUP:
                Events.push_back
                        (createMouseButtonEvent(AVGEvent::MOUSEBUTTONUP, sdlEvent));
                break;
            case SDL_JOYAXISMOTION:
//                Events.push_back(createAxisEvent(sdlEvent));
                break;
            case SDL_JOYBUTTONDOWN:
//                Events.push_back(createButtonEvent(AVGEvent::BUTTON_DOWN, sdlEvent));
                break;
            case SDL_JOYBUTTONUP:
//                Events.push_back(createButtonEvent(AVGEvent::BUTTON_UP, sdlEvent));
                break;
            case SDL_KEYDOWN:
                Events.push_back(createKeyEvent(AVGEvent::KEYDOWN, sdlEvent));
                break;
            case SDL_KEYUP:
                Events.push_back(createKeyEvent(AVGEvent::KEYUP, sdlEvent));
                break;
            case SDL_QUIT:
                {
                    AVGEvent * pEvent = AVGEventDispatcher::createEvent("avgevent");
                    pEvent->init(AVGEvent::QUIT);
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

AVGEvent * AVGSDLDisplayEngine::createMouseMotionEvent
        (int Type, const SDL_Event & SDLEvent)
{
    AVGMouseEvent * pAVGEvent = dynamic_cast<AVGMouseEvent*>
            (AVGEventDispatcher::createEvent("avgmouseevent"));
    pAVGEvent->init(Type, SDLEvent.motion.state & SDL_BUTTON(1),
            SDLEvent.motion.state & SDL_BUTTON(3), 
            SDLEvent.motion.state & SDL_BUTTON(2),
            SDLEvent.motion.x, SDLEvent.motion.y, 
            AVGMouseEvent::NO_BUTTON);
    return pAVGEvent;
}

AVGEvent * AVGSDLDisplayEngine::createMouseButtonEvent
        (int Type, const SDL_Event & SDLEvent) 
{
    AVGMouseEvent * pAVGEvent = dynamic_cast<AVGMouseEvent*>
            (AVGEventDispatcher::createEvent("avgmouseevent"));
    long Button;
    switch (SDLEvent.button.button) {
        case SDL_BUTTON_LEFT:
            Button = AVGMouseEvent::LEFT_BUTTON;
            break;
        case SDL_BUTTON_MIDDLE:
            Button = AVGMouseEvent::MIDDLE_BUTTON;
            break;
        case SDL_BUTTON_RIGHT:
            Button = AVGMouseEvent::RIGHT_BUTTON;
            break;
    }
    pAVGEvent->init(Type, SDLEvent.button.button == SDL_BUTTON_LEFT,
                SDLEvent.button.button == SDL_BUTTON_MIDDLE, 
                SDLEvent.button.button == SDL_BUTTON_RIGHT,
                SDLEvent.button.x, SDLEvent.button.y, Button);
    return pAVGEvent; 
}

/*
AVGEvent * AVGSDLDisplayEngine::createAxisEvent(const SDL_Event & SDLEvent)
{
    return new AVGAxisEvent(SDLEvent.jaxis.which, SDLEvent.jaxis.axis,
                SDLEvent.jaxis.value);
}


AVGEvent * AVGSDLDisplayEngine::createButtonEvent
        (AVGEvent::Type Type, const SDL_Event & SDLEvent) 
{
    return new AVGButtonEvent(Type, SDLEvent.jbutton.which,
                SDLEvent.jbutton.button));
}
*/

AVGEvent * AVGSDLDisplayEngine::createKeyEvent
        (int Type, const SDL_Event & SDLEvent)
{
    long KeyCode = KeyCodeTranslationTable[SDLEvent.key.keysym.sym];
    unsigned int Modifiers = AVGKeyEvent::KEYMOD_NONE;

    if (SDLEvent.key.keysym.mod & KMOD_LSHIFT) 
        { Modifiers |= AVGKeyEvent::KEYMOD_LSHIFT; }
    if (SDLEvent.key.keysym.mod & KMOD_RSHIFT) 
        { Modifiers |= AVGKeyEvent::KEYMOD_RSHIFT; }
    if (SDLEvent.key.keysym.mod & KMOD_LCTRL) 
        { Modifiers |= AVGKeyEvent::KEYMOD_LCTRL; }
    if (SDLEvent.key.keysym.mod & KMOD_RCTRL) 
        { Modifiers |= AVGKeyEvent::KEYMOD_RCTRL; }
    if (SDLEvent.key.keysym.mod & KMOD_LALT) 
        { Modifiers |= AVGKeyEvent::KEYMOD_LALT; }
    if (SDLEvent.key.keysym.mod & KMOD_RALT) 
        { Modifiers |= AVGKeyEvent::KEYMOD_RALT; }
    if (SDLEvent.key.keysym.mod & KMOD_LMETA) 
        { Modifiers |= AVGKeyEvent::KEYMOD_LMETA; }
    if (SDLEvent.key.keysym.mod & KMOD_RMETA) 
        { Modifiers |= AVGKeyEvent::KEYMOD_RMETA; }
    if (SDLEvent.key.keysym.mod & KMOD_NUM) 
        { Modifiers |= AVGKeyEvent::KEYMOD_NUM; }
    if (SDLEvent.key.keysym.mod & KMOD_CAPS) 
        { Modifiers |= AVGKeyEvent::KEYMOD_CAPS; }
    if (SDLEvent.key.keysym.mod & KMOD_MODE) 
        { Modifiers |= AVGKeyEvent::KEYMOD_MODE; }
    if (SDLEvent.key.keysym.mod & KMOD_RESERVED) 
        { Modifiers |= AVGKeyEvent::KEYMOD_RESERVED; }

    AVGKeyEvent * pAVGEvent = dynamic_cast<AVGKeyEvent*>
            (AVGEventDispatcher::createEvent("avgkeyevent"));
    pAVGEvent->init(Type, SDLEvent.key.keysym.scancode, KeyCode,
                SDL_GetKeyName(SDLEvent.key.keysym.sym), Modifiers);
    return pAVGEvent;
}

void AVGSDLDisplayEngine::initJoysticks() 
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


void AVGSDLDisplayEngine::initTranslationTable()
{
#define TRANSLATION_ENTRY(x) KeyCodeTranslationTable[SDLK_##x] = AVGKeyEvent::KEY_##x;

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


