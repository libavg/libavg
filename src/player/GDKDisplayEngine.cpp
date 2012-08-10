
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


#include "GDKDisplayEngine.h"
#include "../avgconfigwrapper.h"

#include "Shape.h"

#include "Event.h"
#include "MouseEvent.h"
#include "KeyEvent.h"
#if defined(HAVE_XI2_1) || defined(HAVE_XI2_2) 
#include "XInputMTInputDevice.h"
#endif
#include "../base/MathHelper.h"
#include "../base/Exception.h"
#include "../base/Logger.h"
#include "../base/ScopeTimer.h"
#include "../base/OSHelper.h"
#include "../base/StringHelper.h"

#include "../graphics/GLContext.h"
#include "../graphics/Filterflip.h"
#include "../graphics/Filterfliprgb.h"

#include "OGLSurface.h"
#include "OffscreenCanvas.h"

#include <gdk/gdkkeysyms.h>

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
#include <math.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

using namespace std;

namespace avg {

float GDKDisplayEngine::s_RefreshRate = 0.0;

GDKDisplayEngine::GDKDisplayEngine()
    : IInputDevice(EXTRACT_INPUTDEVICE_CLASSNAME(GDKDisplayEngine)),
      m_WindowSize(0,0),
      m_ScreenResolution(0,0),
      m_PPMM(0),
      m_pScreen(0),
      m_screen(0),
      m_bMouseOverApp(true),
      m_pLastMouseEvent(new MouseEvent(Event::CURSORMOTION, false, false, false, 
            IntPoint(-1, -1), MouseEvent::NO_BUTTON, glm::vec2(-1, -1), 0)),
      m_NumMouseButtonsDown(0),
      m_noneCursor(0),
      m_cursor(0),
      m_glFullscreenOffset(IntPoint(0,0)),
      m_touchID(0),
      m_multitouch(false)
{
    m_Gamma[0] = 1.0;
    m_Gamma[1] = 1.0;
    m_Gamma[2] = 1.0;

    GdkWindowAttr windowAttr;
    windowAttr.title =  (char*)&"libavg";
    windowAttr.window_type = GDK_WINDOW_TOPLEVEL;
    windowAttr.wclass = GDK_INPUT_OUTPUT;
    windowAttr.width = 100;
    windowAttr.height = 100;

    gdk_init(NULL, NULL);

    m_screen = gdk_screen_get_default();
    m_pScreen = gdk_window_new (NULL, &windowAttr, GDK_WA_TITLE | GDK_WA_X | GDK_WA_Y);

    gdk_window_set_events(m_pScreen, (GdkEventMask) (GDK_POINTER_MOTION_MASK |
            GDK_BUTTON_MOTION_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK |
            GDK_KEY_PRESS_MASK | GDK_KEY_RELEASE_MASK | GDK_SCROLL_MASK | GDK_TOUCH_MASK));

    gdk_window_set_support_multidevice(m_pScreen, true);

    m_noneCursor = gdk_cursor_new(GDK_BLANK_CURSOR);
    gdk_window_show(m_pScreen);
    initTranslationTable();
}

GDKDisplayEngine::~GDKDisplayEngine()
{
    if(m_pScreen) {
        gdk_window_destroy(m_pScreen);
    }
}

void GDKDisplayEngine::init(const DisplayParams& dp, GLConfig glConfig) 
{
    calcScreenDimensions(dp.m_DotsPerMM);
    float aspectRatio = float(dp.m_Size.x)/float(dp.m_Size.y);
    if (dp.m_bFullscreen) {
        int xMax = gdk_screen_get_width(m_screen);
        int yMax =  gdk_screen_get_height(m_screen);
        m_WindowSize.x = xMax;
        m_WindowSize.y = yMax;
        // try scale on x-axis
        m_WindowSize.y = int(m_WindowSize.x / aspectRatio);
        m_glFullscreenOffset.y = (gdk_screen_get_height(m_screen) - m_WindowSize.y) * 0.5;
        if( m_WindowSize.y > yMax) { // test for out of screen
            m_WindowSize.y = yMax;
            m_WindowSize.x = int(m_WindowSize.y * aspectRatio);
            m_glFullscreenOffset.y = 0;
            m_glFullscreenOffset.x = (gdk_screen_get_width(m_screen) - m_WindowSize.x) * 0.5;
        }
        AVG_ASSERT(m_WindowSize.x != 0 && m_WindowSize.y != 0);
        gdk_window_move_resize(m_pScreen, dp.m_Pos.x, dp.m_Pos.y,
                m_WindowSize.x, m_WindowSize.y);
        gdk_window_fullscreen(m_pScreen);
#ifdef __linux
// Fix for broken fullscreen in gdk for mult monitor usage.
        Display* display = gdk_x11_get_default_xdisplay();
        XEvent xev;
        memset(&xev, 0, sizeof(xev));
        xev.type = ClientMessage;
        xev.xclient.window = gdk_x11_window_get_xid(m_pScreen);
        xev.xclient.message_type = XInternAtom(display, "_NET_WM_FULLSCREEN_MONITORS", False);
        xev.xclient.format = 32;
//ToDo: The monitor indices indicating the top, bottom, left, and right edges of the
//      window are from the set returned by the Xinerama extension. 
        xev.xclient.data.l[0] = 0; // your topmost monitor number 
        xev.xclient.data.l[1] = 0; // bottommost 
        xev.xclient.data.l[2] = 1; // leftmost 
        xev.xclient.data.l[3] = 0; // rightmost 
        xev.xclient.data.l[4] = 0; // source indication 

        XSendEvent(display, DefaultRootWindow(display), False,
        SubstructureRedirectMask | SubstructureNotifyMask, &xev);
#endif
    } else {
        if (dp.m_WindowSize == IntPoint(0, 0)) {
            m_WindowSize = dp.m_Size;
        } else if (dp.m_WindowSize.x == 0) {
            m_WindowSize.x = int(dp.m_WindowSize.y*aspectRatio);
            m_WindowSize.y = dp.m_WindowSize.y;
        } else {
            m_WindowSize.x = dp.m_WindowSize.x;
            m_WindowSize.y = int(dp.m_WindowSize.x/aspectRatio);
        }
        AVG_ASSERT(m_WindowSize.x != 0 && m_WindowSize.y != 0);
        gdk_window_move_resize(m_pScreen, dp.m_Pos.x, dp.m_Pos.y,
                m_WindowSize.x, m_WindowSize.y);
    }

    m_bIsFullscreen = dp.m_bFullscreen;

    if (!dp.m_bHasWindowFrame) {
        gdk_window_set_decorations(m_pScreen, (GdkWMDecoration)0);
    }
    bool bAllMultisampleValuesTested = false;
    m_pGLContext = 0;
    while (!bAllMultisampleValuesTested && !m_pGLContext) {
        try {
            m_pGLContext = new GLContext(glConfig, m_pScreen, &dp);
            GLContext::setMain(m_pGLContext);
        } catch (Exception& ex) {
            m_pGLContext = 0;
        }
        if (!m_pGLContext) {
            switch (glConfig.m_MultiSampleSamples) {
                case 1:
                    bAllMultisampleValuesTested = true;
                    break;
                case 2:  
                    glConfig.m_MultiSampleSamples = 1;
                    break;
                case 4:  
                    glConfig.m_MultiSampleSamples = 2;
                    break;
                case 8:  
                    glConfig.m_MultiSampleSamples = 4;
                    break;
                default:
                    glConfig.m_MultiSampleSamples = 8;
                    break;
            }
        }
    }
    if (!m_pGLContext) {
        throw Exception(AVG_ERR_UNSUPPORTED, string("Setting GDK video mode failed: ")
                + ". (size=" + toString(m_WindowSize) + ", bpp=" + 
                toString(dp.m_BPP) + ", multisamplesamples=" + 
                toString(glConfig.m_MultiSampleSamples) + ").");
    }

#if defined(HAVE_XI2_1) || defined(HAVE_XI2_2) 
    m_pXIMTInputDevice = 0;
#endif
    calcRefreshRate();

    glEnable(GL_BLEND);
    GLContext::checkError("init: glEnable(GL_BLEND)");
    glShadeModel(GL_FLAT);
    GLContext::checkError("init: glShadeModel(GL_FLAT)");
    glDisable(GL_DEPTH_TEST);
    GLContext::checkError("init: glDisable(GL_DEPTH_TEST)");
    glEnable(GL_STENCIL_TEST);
    GLContext::checkError("init: glEnable(GL_STENCIL_TEST)");
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE); 
    GLContext::checkError("init: glTexEnvf()");
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glPixelStorei(GL_PACK_ROW_LENGTH, 0);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    if (!queryOGLExtension("GL_ARB_vertex_buffer_object")) {
        throw Exception(AVG_ERR_UNSUPPORTED,
            "Graphics driver lacks vertex buffer support, unable to initialize graphics.");
    }

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnable(GL_TEXTURE_2D);
    setGamma(dp.m_Gamma[0], dp.m_Gamma[1], dp.m_Gamma[2]);
    showCursor(dp.m_bShowCursor);
    if (dp.m_Framerate == 0) {
        setVBlankRate(dp.m_VBRate);
    } else {
        setFramerate(dp.m_Framerate);
    }
    glproc::UseProgramObject(0);
    if (m_pGLContext->useMinimalShader()) {
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glMatrixMode(GL_MODELVIEW);
    }

    m_Size = dp.m_Size;
    m_pGLContext->logConfig();
}

#ifdef _WIN32
#pragma warning(disable: 4996)
#endif
void GDKDisplayEngine::teardown()
{
    if (m_pScreen) {
        if (m_Gamma[0] != 1.0f || m_Gamma[1] != 1.0f || m_Gamma[2] != 1.0f) {
            internalSetGamma(1.0f, 1.0f, 1.0f);
        }
        delete m_pGLContext;
        m_pGLContext = 0;
        GLContext::setMain(0);
    }
}

float GDKDisplayEngine::getRefreshRate() 
{
    if (s_RefreshRate == 0.0) {
        calcRefreshRate();
    }
    return s_RefreshRate;
}

void GDKDisplayEngine::setGamma(float red, float green, float blue)
{
    if (red > 0) {
        AVG_TRACE(Logger::CONFIG, "Setting gamma to " << red << ", " << green << ", "
                << blue);
        bool bOk = internalSetGamma(red, green, blue);
        m_Gamma[0] = red;
        m_Gamma[1] = green;
        m_Gamma[2] = blue;
        if (!bOk) {
            AVG_TRACE(Logger::WARNING, "Unable to set display gamma.");
        }
    }
}

void GDKDisplayEngine::setMousePos(const IntPoint& pos)
{
//    gdk_display_warp_pointer(gdk_window_get_display (m_pScreen), m_screen, pos.x, pos.y);
    gdk_device_warp (gdk_device_manager_get_client_pointer(gdk_display_get_device_manager(
            gdk_display_get_default())), m_screen, pos.x, pos.y);
}

int GDKDisplayEngine::getKeyModifierState() const
{
    int result = key::KEYMOD_NONE;
    unsigned int modi = gdk_keymap_get_modifier_state(gdk_keymap_get_default());

    if (modi & GDK_SHIFT_MASK) {
        result |= key::KEYMOD_SHIFT;
    }
    if (modi & GDK_CONTROL_MASK) {
        result |= key::KEYMOD_CTRL;
    }
    if (modi & GDK_MOD1_MASK) {
        result |= key::KEYMOD_ALT;
    }
    if (modi & GDK_META_MASK) {
        result |= key::KEYMOD_META;
    }
    if (modi & GDK_LOCK_MASK) {
        result |= key::KEYMOD_CAPS;
    }
    return result;

}

void GDKDisplayEngine::calcScreenDimensions(float dotsPerMM)
{
    if (m_ScreenResolution.x == 0) {
        m_ScreenResolution = IntPoint(gdk_screen_get_width(m_screen),
                gdk_screen_get_height(m_screen));
    }
    if (dotsPerMM != 0) {
        m_PPMM = dotsPerMM;
    }

    if (m_PPMM == 0) {
#ifdef WIN32
        HDC hdc = CreateDC("DISPLAY", NULL, NULL, NULL);
        m_PPMM = GetDeviceCaps(hdc, LOGPIXELSX)/25.4f;
#else
    #ifdef linux
        Display * pDisplay = XOpenDisplay(0);
        glm::vec2 displayMM(DisplayWidthMM(pDisplay,0), DisplayHeightMM(pDisplay,0));
    #elif defined __APPLE__
        CGSize size = CGDisplayScreenSize(CGMainDisplayID());
        glm::vec2 displayMM(size.width, size.height);
    #endif
        // Non-Square pixels cause errors here. We'll fix that when it happens.
        m_PPMM = m_ScreenResolution.x/displayMM.x;
#endif
    }
}

bool GDKDisplayEngine::internalSetGamma(float red, float green, float blue)
{
    bool err = false;
#ifdef linux
    XF86VidModeGamma gamma;
    gamma.red = red;
    gamma.green = green;
    gamma.blue = blue;
    err = XF86VidModeSetGamma(gdk_x11_get_default_xdisplay(), gdk_x11_screen_get_screen_number(m_screen), &gamma);
    XF86VidModeGetGamma(gdk_x11_get_default_xdisplay(), gdk_x11_screen_get_screen_number(m_screen), &gamma);
#endif
    return err;
}

static ProfilingZoneID SwapBufferProfilingZone("Render - swap buffers");

void GDKDisplayEngine::swapBuffers()
{
    ScopeTimer timer(SwapBufferProfilingZone);
    m_pGLContext->swapBuffers();
    GLContext::checkError("swapBuffers()");
}

void GDKDisplayEngine::setCursor(GdkPixbuf *pixbuf, int x, int y)
{
    m_cursor = gdk_cursor_new_from_pixbuf(gdk_window_get_display(m_pScreen), pixbuf, x, y);
    gdk_window_set_cursor(m_pScreen, m_cursor);
}

IntPoint GDKDisplayEngine::getGlFullscreenOffset()
{
    return m_glFullscreenOffset;
}

GdkDisplay* GDKDisplayEngine::getDisplay()
{
    return gdk_window_get_display(m_pScreen);
}

void GDKDisplayEngine::enableGDKMultitouchHandling(bool value)
{
    m_multitouch = value;
}

void GDKDisplayEngine::showCursor(bool bShow)
{
#ifdef _WIN32
#define MAX_CORE_POINTERS   6
    // Hack to fix a pointer issue with fullscreen and touchscreens
    // Refer to Mantis bug #140
    for (int i = 0; i < MAX_CORE_POINTERS; ++i) {
        ShowCursor(bShow);
    }
#else
    if (bShow) {
        if (m_cursor != 0){
            gdk_window_set_cursor (m_pScreen, m_cursor);
        } else {
            gdk_window_set_cursor (m_pScreen, NULL);
        }
    } else {
        gdk_window_set_cursor (m_pScreen, m_noneCursor);
    }
#endif
}

BitmapPtr GDKDisplayEngine::screenshot(int buffer)
{
    BitmapPtr pBmp(new Bitmap(m_WindowSize, B8G8R8X8, "screenshot"));
    string sTmp;
    bool bBroken = getEnv("AVG_BROKEN_READBUFFER", sTmp);
    GLenum buf = buffer;
    if (!buffer) {
        if (bBroken) {
            // Workaround for buggy GL_FRONT on some machines.
            buf = GL_BACK;
        } else {
            buf = GL_FRONT;
        }
    }
    glproc::BindFramebuffer(GL_FRAMEBUFFER_EXT, 0);
    glReadBuffer(buf);
    GLContext::checkError("GDKDisplayEngine::screenshot:glReadBuffer()");
    glproc::BindBuffer(GL_PIXEL_PACK_BUFFER_EXT, 0);
    glReadPixels(0, 0, m_WindowSize.x, m_WindowSize.y, GL_BGRA, GL_UNSIGNED_BYTE, 
            pBmp->getPixels());
    GLContext::checkError("GDKDisplayEngine::screenshot:glReadPixels()");
    FilterFlip().applyInPlace(pBmp);
    return pBmp;
}

IntPoint GDKDisplayEngine::getSize()
{
    return m_Size;
}

void GDKDisplayEngine::calcRefreshRate()
{
    float lastRefreshRate = s_RefreshRate;
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
    HDC hDC = CreateDC("DISPLAY", NULL, NULL, NULL);
    s_RefreshRate = float(GetDeviceCaps(hDC, VREFRESH));
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
    float HSyncRate = pixelClock*1000.0/modeLine.htotal;
    s_RefreshRate = HSyncRate/modeLine.vtotal;
    XCloseDisplay(pDisplay);
#endif
    if (s_RefreshRate == 0) {
        s_RefreshRate = 60;
    }
    if (lastRefreshRate != s_RefreshRate) {
        AVG_TRACE(Logger::CONFIG, "Vertical Refresh Rate: " << s_RefreshRate);
    }

}

vector<long> GDKDisplayEngine::KeyCodeTranslationTable(GDK_KEY_VoidSymbol, key::KEY_UNKNOWN);

const char * getEventTypeName(unsigned char type) 
{
    switch (type) {
            case GDK_ENTER_NOTIFY:
                return "GDK_ENTER_NOTIFY";
            case GDK_LEAVE_NOTIFY:
                return "GDK_LEAVE_NOTIFY";
            case GDK_KEY_PRESS:
                return "GDK_KEY_PRESS";
            case GDK_KEY_RELEASE:
                return "GDK_KEY_RELEASE";
            case GDK_MOTION_NOTIFY:
                return "GDK_MOTION_NOTIFY";
            case GDK_BUTTON_PRESS:
                return "GDK_BUTTON_PRESS";
            case GDK_BUTTON_RELEASE:
                return "GDK_BUTTON_RELEASE";
            case GDK_SCROLL:
                return "GDK_SCROLL";
            case GDK_CONFIGURE:
                return "GDK_CONFIGURE";
            case GDK_DESTROY:
                return "GDK_DELETE";
            default:
                return "Unknown GDK event type";
    }
}

vector<EventPtr> GDKDisplayEngine::pollEvents()
{
    GdkEvent* gdkEvent;
    vector<EventPtr> events;
    while(gdk_events_pending()) {
        gdkEvent = gdk_event_get();
        if(gdkEvent != NULL) {
            EventPtr pNewEvent;
            switch (gdkEvent->type) {
                case GDK_MOTION_NOTIFY:
                    if (m_bMouseOverApp) {
                        pNewEvent = createMouseEvent(Event::CURSORMOTION, *gdkEvent,
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
                case GDK_BUTTON_PRESS:
                    pNewEvent = createMouseButtonEvent(Event::CURSORDOWN, *gdkEvent);
                    break;
                case GDK_BUTTON_RELEASE:
                    pNewEvent = createMouseButtonEvent(Event::CURSORUP, *gdkEvent);
                    break;
                case GDK_SCROLL:
                    pNewEvent = createMouseWheelEvent(Event::CUSTOMEVENT, *gdkEvent);
                    break;
                case GDK_KEY_PRESS:
                    pNewEvent = createKeyEvent(Event::KEYDOWN, *gdkEvent);
                    break;
                case GDK_KEY_RELEASE:
                    pNewEvent = createKeyEvent(Event::KEYUP, *gdkEvent);
                    break;
                case GDK_DELETE:
                    pNewEvent = EventPtr(new Event(Event::QUIT, Event::NONE));
                    break;

                default:
                    // Ignore unknown events.
                    break;
            }
            if (m_multitouch) {
                switch (gdkEvent->type) {
                    case GDK_TOUCH_BEGIN:
                        {
                        m_touchID++;
                        TouchEventPtr newEvent = createTouchEvent(m_touchID, Event::CURSORDOWN, *gdkEvent);
                        m_pXIMTInputDevice->addTouchStatusViaSeq(gdk_event_get_event_sequence(gdkEvent), newEvent);
                        break;
                        }
                    case GDK_TOUCH_UPDATE:
                        {
                        TouchEventPtr newEvent = createTouchEvent(0, Event::CURSORMOTION, *gdkEvent);
                        TouchStatusPtr pTouchStatus = m_pXIMTInputDevice->getTouchStatusViaSeq(gdk_event_get_event_sequence(gdkEvent));
                        AVG_ASSERT(pTouchStatus);
                        pTouchStatus->pushEvent(newEvent);
                        break;
                        }
                    case GDK_TOUCH_END:
                        {
                        TouchEventPtr newEvent = createTouchEvent(0, Event::CURSORUP, *gdkEvent);
                        TouchStatusPtr pTouchStatus = m_pXIMTInputDevice->getTouchStatusViaSeq(gdk_event_get_event_sequence(gdkEvent));
                        pTouchStatus->pushEvent(newEvent);
                        break;
                        }
                    case GDK_TOUCH_CANCEL:
                        AVG_ASSERT(False);
                        break;

                    default:
                    // Ignore unknown events.
                    break;
                }
            }
            if (pNewEvent) {
                events.push_back(pNewEvent);
            } 
            gdk_event_free(gdkEvent);
        }
    }
    return events;
}

void GDKDisplayEngine::setXIMTInputDevice(XInputMTInputDevice* pInputDevice)
{
    AVG_ASSERT(!m_pXIMTInputDevice);
    m_pXIMTInputDevice = pInputDevice;
}

EventPtr GDKDisplayEngine::createMouseEvent(Event::Type type, const GdkEvent& gdkEvent,
        long button)
{
    int x = ((GdkEventMotion&)gdkEvent).x;
    int y = ((GdkEventMotion&)gdkEvent).y;
    x = int(((x-m_glFullscreenOffset.x)*m_Size.x)/m_WindowSize.x);
    y = int(((y-m_glFullscreenOffset.y)*m_Size.y)/m_WindowSize.y);
    glm::vec2 lastMousePos = m_pLastMouseEvent->getPos();
    glm::vec2 speed;
    if (lastMousePos.x == -1) {
        speed = glm::vec2(0,0);
    } else {
        float lastFrameTime = 1000/getEffectiveFramerate();
        speed = glm::vec2(x-lastMousePos.x, y-lastMousePos.y)/lastFrameTime;
    }
     MouseEventPtr pEvent;
    if(gdkEvent.type == GDK_MOTION_NOTIFY) {
        pEvent = MouseEventPtr(new MouseEvent(type, false, false, false, IntPoint(x, y),
                button, speed));
    } else {
        GdkEventButton event = (GdkEventButton&)gdkEvent;
        pEvent = MouseEventPtr(new MouseEvent(type, event.button == 1, event.button == 3,
                event.button == 2, IntPoint(x, y), button, speed));
    }
    m_pLastMouseEvent = pEvent;
    return pEvent; 
}

EventPtr GDKDisplayEngine::createMouseButtonEvent(Event::Type type,
        const GdkEvent& gdkEvent)
{
    long button = 0;
    switch (((GdkEventButton&)gdkEvent).button) {
        case 1:
            button = MouseEvent::LEFT_BUTTON;
            break;
        case 2:
            button = MouseEvent::RIGHT_BUTTON;
            break;
        case 3:
            button = MouseEvent::MIDDLE_BUTTON;
            break;
    }
    return createMouseEvent(type, gdkEvent, button);
}

// to observe
EventPtr GDKDisplayEngine::createMouseWheelEvent(Event::Type type,
        const GdkEvent& gdkEvent)
{
    long button = 0;
    if(((GdkEventScroll&)gdkEvent).delta_y > 0) {
        button = MouseEvent::WHEELUP_BUTTON;
    } else {
        button = MouseEvent::WHEELDOWN_BUTTON;
    }
    return createMouseEvent(type, gdkEvent, button);
}

EventPtr GDKDisplayEngine::createKeyEvent(Event::Type type, const GdkEvent& gdkEvent)
{
    GdkEventKey keyEvent = (GdkEventKey&) gdkEvent;

    unsigned int keyVal = keyEvent.keyval;

    long keyCode = KeyCodeTranslationTable[keyVal];
    unsigned int modifiers = key::KEYMOD_NONE;

    if (keyEvent.state & GDK_SHIFT_MASK) 
        { modifiers |= key::KEYMOD_SHIFT; }

    if (keyEvent.state & GDK_CONTROL_MASK) 
        { modifiers |= key::KEYMOD_CTRL; }

    if (keyEvent.state & GDK_MOD1_MASK) 
        { modifiers |= key::KEYMOD_ALT; }

    if (keyEvent.state & GDK_META_MASK) 
        { modifiers |= key::KEYMOD_META; }

    if (keyEvent.state & GDK_LOCK_MASK) 
        { modifiers |= key::KEYMOD_CAPS; }

/*    cout << type << " " << gdk_keyval_name(keyVal) << endl;
    cout << "hard|code:  " << keyEvent.hardware_keycode << "|" << keyCode << endl;
    cout << "Mod: " << modifiers << endl;
    cout << endl; */

    KeyEventPtr pEvent(new KeyEvent(type, keyEvent.hardware_keycode, keyCode,
            gdk_keyval_name(gdk_keyval_to_lower(keyVal)), gdk_keyval_to_unicode(keyVal),
            modifiers));
    return pEvent;
}

TouchEventPtr GDKDisplayEngine::createTouchEvent(int id, Event::Type type, const GdkEvent &gdkEvent)
{
    GdkEventTouch touchEvent = (GdkEventTouch&) gdkEvent;
    int x = int(((touchEvent.x-m_glFullscreenOffset.x)*m_Size.x)/m_WindowSize.x);
    int y = int(((touchEvent.y-m_glFullscreenOffset.y)*m_Size.y)/m_WindowSize.y);
    IntPoint pos(x,y);
    TouchEventPtr pEvent(new TouchEvent(id, type, pos, Event::TOUCH));
    return pEvent;
}

void GDKDisplayEngine::initTranslationTable()
{
#define TRANSLATION_ENTRY(x) KeyCodeTranslationTable[GDK_KEY_##x] = key::KEY_##x;

//    TRANSLATION_ENTRY(UNKNOWN);
    TRANSLATION_ENTRY(BackSpace);
    TRANSLATION_ENTRY(Tab);
    TRANSLATION_ENTRY(Clear);
    TRANSLATION_ENTRY(Return);
    TRANSLATION_ENTRY(Pause);
    TRANSLATION_ENTRY(Escape);
    TRANSLATION_ENTRY(space);
    TRANSLATION_ENTRY(Delete);

    TRANSLATION_ENTRY(exclam);
    TRANSLATION_ENTRY(quotedbl);
    TRANSLATION_ENTRY(numbersign);
    TRANSLATION_ENTRY(dollar);
    TRANSLATION_ENTRY(ampersand);
    TRANSLATION_ENTRY(apostrophe);
    TRANSLATION_ENTRY(parenleft);
    TRANSLATION_ENTRY(parenright);
    TRANSLATION_ENTRY(asterisk);
    TRANSLATION_ENTRY(plus);
    TRANSLATION_ENTRY(comma);
    TRANSLATION_ENTRY(minus);
    TRANSLATION_ENTRY(period);
    TRANSLATION_ENTRY(slash);
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
    TRANSLATION_ENTRY(colon);
    TRANSLATION_ENTRY(semicolon);
    TRANSLATION_ENTRY(less);
    TRANSLATION_ENTRY(equal);
    TRANSLATION_ENTRY(greater);
    TRANSLATION_ENTRY(question);
    TRANSLATION_ENTRY(at);
    TRANSLATION_ENTRY(bracketleft);
    TRANSLATION_ENTRY(backslash);
    TRANSLATION_ENTRY(bracketright);
    TRANSLATION_ENTRY(caret);
    TRANSLATION_ENTRY(underscore);
    TRANSLATION_ENTRY(dead_grave);
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
    TRANSLATION_ENTRY(KP_0);
    TRANSLATION_ENTRY(KP_1);
    TRANSLATION_ENTRY(KP_2);
    TRANSLATION_ENTRY(KP_3);
    TRANSLATION_ENTRY(KP_4);
    TRANSLATION_ENTRY(KP_5);
    TRANSLATION_ENTRY(KP_6);
    TRANSLATION_ENTRY(KP_7);
    TRANSLATION_ENTRY(KP_8);
    TRANSLATION_ENTRY(KP_9);
    TRANSLATION_ENTRY(KP_Separator);
    TRANSLATION_ENTRY(KP_Divide);
    TRANSLATION_ENTRY(KP_Multiply);
    TRANSLATION_ENTRY(KP_Subtract);
    TRANSLATION_ENTRY(KP_Add);
    TRANSLATION_ENTRY(KP_Enter);
    TRANSLATION_ENTRY(KP_Equal);
    TRANSLATION_ENTRY(Up);
    TRANSLATION_ENTRY(Down);
    TRANSLATION_ENTRY(Right);
    TRANSLATION_ENTRY(Left);
    TRANSLATION_ENTRY(Insert);
    TRANSLATION_ENTRY(Home);
    TRANSLATION_ENTRY(End);
    TRANSLATION_ENTRY(Page_Up);
    TRANSLATION_ENTRY(Page_Down);
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
    TRANSLATION_ENTRY(Num_Lock);
    TRANSLATION_ENTRY(Caps_Lock);
    TRANSLATION_ENTRY(Scroll_Lock);
    TRANSLATION_ENTRY(Shift_R);
    TRANSLATION_ENTRY(Shift_L);
    TRANSLATION_ENTRY(Control_R);
    TRANSLATION_ENTRY(Control_L);
    TRANSLATION_ENTRY(Alt_R);
    TRANSLATION_ENTRY(Alt_L);
    TRANSLATION_ENTRY(Meta_R);
    TRANSLATION_ENTRY(Meta_L);
    TRANSLATION_ENTRY(Super_L);
    TRANSLATION_ENTRY(Super_R);
    TRANSLATION_ENTRY(ISO_Level3_Shift);
    TRANSLATION_ENTRY(Help);
    TRANSLATION_ENTRY(Print);
    TRANSLATION_ENTRY(Sys_Req);
    TRANSLATION_ENTRY(Break);
    TRANSLATION_ENTRY(Menu);
    TRANSLATION_ENTRY(EuroSign);
    TRANSLATION_ENTRY(Undo);

    TRANSLATION_ENTRY(ssharp);
    TRANSLATION_ENTRY(adiaeresis);
    TRANSLATION_ENTRY(odiaeresis);
    TRANSLATION_ENTRY(udiaeresis);
    TRANSLATION_ENTRY(dead_circumflex);
    TRANSLATION_ENTRY(degree);
    TRANSLATION_ENTRY(section);
    TRANSLATION_ENTRY(percent);
    TRANSLATION_ENTRY(dead_acute);
    TRANSLATION_ENTRY(bar);
}

const IntPoint& GDKDisplayEngine::getWindowSize() const
{
    return m_WindowSize;
}

bool GDKDisplayEngine::isFullscreen() const
{
    return m_bIsFullscreen;
}

IntPoint GDKDisplayEngine::getScreenResolution()
{
    calcScreenDimensions();
    return m_ScreenResolution;
}

float GDKDisplayEngine::getPixelsPerMM()
{
    calcScreenDimensions();

    return m_PPMM;
}

glm::vec2 GDKDisplayEngine::getPhysicalScreenDimensions()
{
    calcScreenDimensions();
    glm::vec2 size;
    glm::vec2 screenRes = glm::vec2(getScreenResolution());
    size.x = screenRes.x/m_PPMM;
    size.y = screenRes.y/m_PPMM;
    return size;
}

void GDKDisplayEngine::assumePixelsPerMM(float ppmm)
{
    m_PPMM = ppmm;
}

}
