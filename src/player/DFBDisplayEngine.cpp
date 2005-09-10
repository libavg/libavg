//
// $Id$
//

#include "DFBDisplayEngine.h"
#include "Region.h"
#include "Player.h"
#include "Node.h"
#include "AVGNode.h"
#include "FramerateManager.h"
#include "DFBSurface.h"

#include "Event.h"
#include "MouseEvent.h"
#include "KeyEvent.h"

#include "../base/Exception.h"
#include "../base/Logger.h"

#include <directfb.h>
#include <signal.h>

#include <iostream>
#include <sstream>
#include <unistd.h>
#include <assert.h>

using namespace std;

namespace avg {

DFBDisplayEngine::DFBDisplayEngine()
    : m_pDirectFB(0),
      m_pBackBuffer(0)
{
}

DFBDisplayEngine::~DFBDisplayEngine()
{
    if (m_pDirectFB) {
        teardown();
    }
}

void DFBDisplayEngine::dumpSurface (IDirectFBSurface * pSurf, const string & name)
{
    int w, h;
    AVG_TRACE(Logger::BLTS, "Surface: " << name);

    pSurf->GetSize(pSurf, &w, &h);
    AVG_TRACE(Logger::BLTS, "  Size: " << w << "x" << h);

    DFBRectangle rect;
    pSurf->GetVisibleRectangle(pSurf, &rect);
    AVG_TRACE(Logger::BLTS, "  VisibleRect: x: " << rect.x << 
            ", y: " << rect.y << ", w: " << rect.w << ", h: " << rect.h);

    DFBSurfaceCapabilities caps;
    pSurf->GetCapabilities(pSurf, &caps);
    AVG_TRACE(Logger::BLTS, "  Caps: " << std::hex << caps);

    DFBSurfacePixelFormat fmt;
    pSurf->GetPixelFormat(pSurf, &fmt);
    string sFmt;
    switch (fmt) {
        case DSPF_ARGB1555:
            sFmt = "DSPF_ARGB155";
            break;
        case DSPF_RGB16:
            sFmt = "DSPF_RGB16";
            break;
        case DSPF_RGB24:
            sFmt = "DSPF_RGB24";
            break;
        case DSPF_RGB32:
            sFmt = "DSPF_RGB32";
            break;
        case DSPF_ARGB:
            sFmt = "DSPF_ARGB";
            break;
        case DSPF_A8:
            sFmt = "DSPF_A8";
            break;
        case DSPF_YUY2:
            sFmt = "DSPF_YUY2";
            break;
        case DSPF_RGB332:
            sFmt = "DSPF_RGB332";
            break;
        case DSPF_UYVY:
            sFmt = "DSPF_UYVY";
            break;
        case DSPF_I420:
            sFmt = "DSPF_I420";
            break;
        case DSPF_YV12:
            sFmt = "DSPF_YV12";
            break;
        case DSPF_LUT8:
            sFmt = "DSPF_LUT8";
            break;
        case DSPF_ALUT44:
            sFmt = "DSPF_ALUT44";
            break;
/*        case DSPF_AiRGB:
            sFmt = "DSPF_AiRGB";
            break;
        case DSPF_A1:
            sFmt = "DSPF_A1";
            break;*/
        default:
            sFmt = ""; 
    }
    if (sFmt != "") {
        AVG_TRACE(Logger::BLTS, "  PixelFormat: " << sFmt);
    } else {
        AVG_TRACE(Logger::BLTS, "  PixelFormat: " << fmt << std::dec);
    }
}


void DFBDisplayEngine::init(int width, int height, bool isFullscreen, int bpp,
        int WindowWidth, int WindowHeight)
{
    if (WindowWidth != 0 || WindowHeight != 0) {
        AVG_TRACE(Logger::ERROR, 
                "Can't set window width or height in DFB renderer. Aborting.");
        exit(-1);
    }
    DFBResult err;

    if (m_pDirectFB) {
        teardown();
    }

    initDFB(width, height, isFullscreen, bpp);
    initLayer(width, height);
    
    initBackbuffer();
    
    initInput();

    IDirectFBSurface * pLayerSurf;
    err = m_pDFBLayer->GetSurface(m_pDFBLayer, &pLayerSurf);
    DFBErrorCheck(AVG_ERR_VIDEO_INIT_FAILED, "DFBDisplayEngine::init", err);
    dumpSurface (pLayerSurf, "Layer surface");

    dumpSurface (m_pBackBuffer, "Backbuffer surface");

    pLayerSurf->Release(pLayerSurf);
    
    // SDL sets up a signal handler we really don't want.
    if (!m_IsFullscreen) {
        signal(SIGSEGV, SIG_DFL);
    }
}

void DFBDisplayEngine::initDFB(int width, int height, 
        bool isFullscreen, int bpp)
{
    // Init DFB system
    char ** argv = new (char *)[7];
    int argc = 3;
    argv[0] = strdup ("bogus_appname");
    argv[1] = strdup("--dfb:no-banner");
    argv[2] = strdup("--dfb:quiet");
    
    if (isFullscreen && geteuid() != 0) {
        isFullscreen = false;
        AVG_TRACE(Logger::PROFILE, 
                "Fullscreen requested but not running as root.");
        AVG_TRACE(Logger::PROFILE, 
                "         Falling back to windowed mode.");
    }
    
    if (!isFullscreen) {
        argc = 7;
        char tmp[256];
        sprintf(tmp, "--dfb:mode=%ix%i", width, height);
        argv[3] = strdup (tmp);
        if (bpp == 16 || bpp == 15) {
            argv[4] = strdup ("--dfb:pixelformat=RGB16");
        } else {
            argv[4] = strdup ("--dfb:pixelformat=RGB24");
        }
        argv[5] = strdup ("--dfb:force-windowed");
        argv[6] = strdup ("--dfb:system=SDL");
    }

    DFBResult err;
    err = DirectFBInit (&argc, &argv);
    DFBErrorCheck(AVG_ERR_VIDEO_INIT_FAILED, 
            "DFBDisplayEngine::initDFB - DirectFBInit", err);
    err = DirectFBCreate (&m_pDirectFB);
    DFBErrorCheck(AVG_ERR_VIDEO_INIT_FAILED, 
            "DFBDisplayEngine::initDFB - DirectFBCreate", err);
    
    m_IsFullscreen = isFullscreen;
    m_bpp = bpp;
    DFBSurface::SetDirectFB(m_pDirectFB);
}

void DFBDisplayEngine::initLayer(int width, int height)
{
    DFBResult err;
    err = m_pDirectFB->GetDisplayLayer(m_pDirectFB, DLID_PRIMARY, &m_pDFBLayer);
    DFBErrorCheck(AVG_ERR_DFB, "DFBDisplayEngine::init", err);
    DFBDisplayLayerDescription LayerDesc;
    err = m_pDFBLayer->GetDescription(m_pDFBLayer, &LayerDesc);
    DFBErrorCheck(AVG_ERR_DFB, "DFBDisplayEngine::init", err);
    assert(int(LayerDesc.type) && int(DLTF_GRAPHICS) == int(DLTF_GRAPHICS));
    
    DFBDisplayLayerConfig LayerConfig;
    err = m_pDFBLayer->GetConfiguration(m_pDFBLayer, &LayerConfig);
    DFBErrorCheck(AVG_ERR_DFB, "DFBDisplayEngine::init", err);
    m_Width = LayerConfig.width;
    m_Height = LayerConfig.height;
    if (width != m_Width || height != m_Height) {
        cerr << "Warning: avg file expects screen dimensions of " << 
            width << "x" << height << "." << endl;
        cerr << "         Current resolution is " << m_Width 
            << "x" << m_Height << endl;
        cerr << "         To avoid this, change dfb configuration." << endl;
    }

    err = m_pDFBLayer->SetCooperativeLevel(m_pDFBLayer, 
            DFBDisplayLayerCooperativeLevel(DLSCL_ADMINISTRATIVE));
    DFBErrorCheck(AVG_ERR_DFB, "DFBDisplayEngine::init", err);
 
    if (m_IsFullscreen) {
        LayerConfig.flags = DLCONF_BUFFERMODE;
        LayerConfig.buffermode = DLBM_FRONTONLY;  
        err = m_pDFBLayer->SetConfiguration(m_pDFBLayer, &LayerConfig);
        DFBErrorCheck(AVG_ERR_DFB, "DFBDisplayEngine::init", err);

    }
    err = m_pDFBLayer->EnableCursor(m_pDFBLayer, true);
    DFBErrorCheck(AVG_ERR_DFB, "DFBDisplayEngine::init", err);
    showCursor(false);
}


void DFBDisplayEngine::initInput() {
    DFBResult err;
    // Init window
    DFBWindowDescription WinDesc;
    WinDesc.flags = DFBWindowDescriptionFlags(DWDESC_CAPS | DWDESC_WIDTH |             
            DWDESC_HEIGHT | DWDESC_POSX | DWDESC_POSY);
    WinDesc.caps = DFBWindowCapabilities(DWCAPS_INPUTONLY);
    WinDesc.width = m_Width;
    WinDesc.height = m_Height;
    WinDesc.posx = 0;
    WinDesc.posy = 0;
    err = m_pDFBLayer->CreateWindow(m_pDFBLayer, &WinDesc, &m_pDFBWindow);
    DFBErrorCheck(AVG_ERR_DFB, "DFBDisplayEngine::initInput CreateWindow", err);

    err = m_pDFBWindow->CreateEventBuffer (m_pDFBWindow, &m_pEventBuffer);
    DFBErrorCheck (AVG_ERR_DFB, "DFBDisplayEngine::initInput CreateEventBuffer", err);
    err = m_pDFBWindow->EnableEvents (m_pDFBWindow, DWET_ALL);
    DFBErrorCheck (AVG_ERR_DFB, "DFBDisplayEngine::initInput EnableEvents", err);

    err = m_pDFBWindow->GrabKeyboard(m_pDFBWindow);
    DFBErrorCheck (AVG_ERR_DFB, "DFBDisplayEngine::initInput GrabKeyboard", err);
    err = m_pDFBWindow->GrabPointer(m_pDFBWindow);
    DFBErrorCheck (AVG_ERR_DFB, "DFBDisplayEngine::initInput GrabPointer", err);
}

void DFBDisplayEngine::initBackbuffer()
{
    DFBResult err;
    DFBSurfaceDescription Description;
    Description.flags = DFBSurfaceDescriptionFlags
            (DSDESC_CAPS | DSDESC_WIDTH | DSDESC_HEIGHT | DSDESC_PIXELFORMAT);
    Description.caps =  DSCAPS_SYSTEMONLY;
    Description.width = m_Width;
    Description.height = m_Height;
    if (m_bpp == 16) {
        Description.pixelformat = DSPF_RGB16; 
    } else {
        Description.pixelformat = DSPF_RGB24; 
    }
    err = m_pDirectFB->CreateSurface(m_pDirectFB, &Description, &m_pBackBuffer);
    DFBErrorCheck (AVG_ERR_DFB, "DFBDisplayEngine::initBackbuffer", err);
}

void DFBDisplayEngine::teardown()
{
    m_pEventBuffer->Release(m_pEventBuffer);
    m_pEventBuffer = 0;
    
    m_pBackBuffer->Release(m_pBackBuffer);
    m_pBackBuffer = 0;
    
    m_pDFBWindow->Close(m_pDFBWindow);
    m_pDFBWindow->Destroy(m_pDFBWindow);
    m_pDFBWindow->Release(m_pDFBWindow);
    m_pDFBWindow = 0;
    
    m_pDFBLayer->Release(m_pDFBLayer);
    m_pDirectFB->Release(m_pDirectFB);
    m_pDirectFB = 0;
}

void DFBDisplayEngine::render(AVGNode * pRootNode, 
        FramerateManager * pFramerateManager, bool bRenderEverything)
{
    pRootNode->prepareRender(0, pRootNode->getAbsViewport());
    Region UpdateRegion;
    if (bRenderEverything) {
        DRect rc(0,0, m_Width, m_Height);
        UpdateRegion.addRect(rc);
    } else {
        pRootNode->getDirtyRegion(UpdateRegion);
    }
//    UpdateRegion.dump();
    for (int i = 0; i<UpdateRegion.getNumRects(); i++) {
        const DRect & rc = UpdateRegion.getRect(i);
        setDirtyRect(rc);
        setClipRect();
        clear();
        pRootNode->maybeRender(rc);
    }
    pFramerateManager->FrameWait();
    swapBuffers(UpdateRegion);
    pFramerateManager->CheckJitter();
}

void DFBDisplayEngine::setClipRect()
{
    pushClipRect(DRect(0, 0, m_Width, m_Height), true);
}

bool DFBDisplayEngine::pushClipRect(const DRect& rc, bool bClip)
{
    m_ClipRect = rc;
    m_ClipRect.Intersect(m_DirtyRect);
    if (m_ClipRect.Width() > 0 && m_ClipRect.Height() > 0) {
        DFBRegion Region;
        Region.x1 = (int)(m_ClipRect.tl.x+0.5);
        Region.y1 = (int)(m_ClipRect.tl.y+0.5);
        Region.x2 = (int)(m_ClipRect.br.x-0.5);
        Region.y2 = (int)(m_ClipRect.br.y-0.5);
        m_pBackBuffer->SetClip(m_pBackBuffer, &Region);
        AVG_TRACE(Logger::BLTS, "Clip set to " << 
                m_ClipRect.tl.x << "x" << m_ClipRect.tl.y << 
                ", width: " << m_ClipRect.Width() << ", height: " << 
                m_ClipRect.Height());
        return true;
    } else {
        return false;
    }
}

void DFBDisplayEngine::popClipRect()
{
}

const DRect& DFBDisplayEngine::getClipRect() {
    return m_ClipRect;
}

void DFBDisplayEngine::blt32(ISurface * pSurface, 
        const DRect* pDestRect, 
        double opacity, double angle, const DPoint& pivot, BlendMode Mode)
{
    DFBSurface * pDFBSurface = 
            dynamic_cast<DFBSurface *>(pSurface);
    assert(pDFBSurface); // Make sure we have the correct type of surface
    IDirectFBSurface * pSurf = pDFBSurface->getSurface();
    blt32(pSurf, pDestRect, opacity, (pSurface->getBmp()->getPixelFormat() == B8G8R8A8),
            Mode);
}

void DFBDisplayEngine::blta8(ISurface * pSurface, 
        const DRect* pDestRect, 
        double opacity, const Pixel32& color, double angle, 
        const DPoint& pivot, BlendMode Mode)
{
    m_pBackBuffer->SetColor(m_pBackBuffer, 
            color.getR(), color.getG(), color.getB(),
            __u8(opacity*256));

    DFBSurfaceBlittingFlags BltFlags;
    BltFlags = DFBSurfaceBlittingFlags(DSBLIT_BLEND_ALPHACHANNEL | 
                                       DSBLIT_COLORIZE);
    m_pBackBuffer->SetBlittingFlags(m_pBackBuffer, BltFlags);
    DFBSurface * pDFBSurface = 
            dynamic_cast<DFBSurface *>(pSurface);
    assert(pDFBSurface); // Make sure we have the correct type of surface
    IDirectFBSurface * pSurf = pDFBSurface->getSurface();

    blt(pSurf, pDestRect);
}

void DFBDisplayEngine::blt32(IDirectFBSurface * pSrc,  
        const DRect* pDestRect, double opacity, bool bAlpha, BlendMode Mode)
{
    DFBSurfaceBlittingFlags BltFlags;
    if (bAlpha) {
        BltFlags = DSBLIT_BLEND_ALPHACHANNEL;
    } else {
        BltFlags = DSBLIT_NOFX;
    }
    if (opacity < 0.9999) {
        BltFlags = DFBSurfaceBlittingFlags(BltFlags | DSBLIT_BLEND_COLORALPHA);
        m_pBackBuffer->SetColor(m_pBackBuffer, 
                             0xff, 0xff, 0xff, __u8(opacity*256));
    }
    m_pBackBuffer->SetBlittingFlags(m_pBackBuffer, BltFlags);
    switch(Mode) {
        case BLEND_ADD:
            m_pBackBuffer->SetSrcBlendFunction(m_pBackBuffer, DSBF_SRCALPHA);
            m_pBackBuffer->SetDstBlendFunction
                    (m_pBackBuffer, DSBF_ONE);
            break;
        case BLEND_BLEND:
        default:
            m_pBackBuffer->SetSrcBlendFunction(m_pBackBuffer, DSBF_SRCALPHA);
            m_pBackBuffer->SetDstBlendFunction
                    (m_pBackBuffer, DSBF_INVSRCALPHA);
            break;
    }
//    dumpSurface (pSrc, "pSrc");
//    dumpSurface (m_pBackBuffer, "m_pBackBuffer");
    blt(pSrc, pDestRect);
}
 
// Assumes blit flags and colors are already set.
void DFBDisplayEngine::blt(IDirectFBSurface * pSrc, const DRect* pDestRect)
{
    int width;
    int height;
    pSrc->GetSize(pSrc, &width, &height);

    if (m_ClipRect.br.x < pDestRect->tl.x || m_ClipRect.br.y < pDestRect->tl.y) {
        return;
    }
    AVG_TRACE(Logger::BLTS, "Blit: (" << pDestRect->tl.x << 
            ", " << pDestRect->tl.y << 
            "), width:" << width << ", height: " << height);
    DFBRectangle DFBDestRect;
    DFBDestRect.x = int(pDestRect->tl.x+0.5);
    DFBDestRect.y = int(pDestRect->tl.y+0.5);
    DFBDestRect.w = int(pDestRect->Width()+0.5);
    DFBDestRect.h = int(pDestRect->Height()+0.5);


    DFBResult err = m_pBackBuffer->StretchBlit(m_pBackBuffer, pSrc, 0, 
            &DFBDestRect);
        
    DFBErrorCheck(AVG_ERR_VIDEO_GENERAL, "DFBDisplayEngine::blt", err);
}

void DFBDisplayEngine::clear()
{
    DFBResult err;
    m_pBackBuffer->SetDrawingFlags(m_pBackBuffer, DSDRAW_NOFX);
    m_pBackBuffer->SetColor(m_pBackBuffer, 0x0, 0x00, 0x00, 0xff);
    AVG_TRACE(Logger::BLTS, "Clear rect: " << m_DirtyRect.tl.x << "x" << 
            m_DirtyRect.tl.y << ", width: " << m_DirtyRect.Width() << 
            ", height: " << m_DirtyRect.Height());
 
    if (m_DirtyRect.Width() > 0 && m_DirtyRect.Height() > 0) {   
        err = m_pBackBuffer->FillRectangle(m_pBackBuffer, 
                int(m_DirtyRect.tl.x+0.5), int(m_DirtyRect.tl.y+0.5), 
                int(m_DirtyRect.Width()+0.5), int(m_DirtyRect.Height()+0.5));
        DFBErrorCheck(AVG_ERR_VIDEO_GENERAL, "DFBDisplayEngine::clear", err);
    }
}

void DFBDisplayEngine::setDirtyRect(const DRect& rc) 
{
    m_DirtyRect = rc;

    AVG_TRACE(Logger::BLTS, "Dirty rect: " << m_DirtyRect.tl.x << "x" << 
            m_DirtyRect.tl.y << ", width: " << m_DirtyRect.Width() << 
            ", height: " << m_DirtyRect.Height());
}

void DFBDisplayEngine::swapBuffers(const Region & UpdateRegion)
{
    DFBResult err;
    IDirectFBSurface * pLayerSurf;
    err = m_pDFBLayer->GetSurface(m_pDFBLayer, &pLayerSurf);
    DFBErrorCheck(AVG_ERR_VIDEO_GENERAL, "DFBDisplayEngine::swapBuffers", err);
    pLayerSurf->SetBlittingFlags(pLayerSurf, DSBLIT_NOFX);
//    m_pDirectFB->WaitForSync(m_pDirectFB);
    for (int i = 0; i<UpdateRegion.getNumRects(); i++) {
        const DRect & rc = UpdateRegion.getRect(i);
        DFBRectangle DFBRect;
        DFBRect.x = int(rc.tl.x+0.5);
        DFBRect.y = int(rc.tl.y+0.5);
        DFBRect.w = int(rc.Width()+0.5);
        DFBRect.h = int(rc.Height()+0.5);
        err = pLayerSurf->Blit(pLayerSurf, m_pBackBuffer, &DFBRect, 
                int(rc.tl.x+0.5), int(rc.tl.y+0.5));
        DFBErrorCheck(AVG_ERR_VIDEO_GENERAL, "DFBDisplayEngine::swapBuffers", err);

        int width;
        int height;
        m_pBackBuffer->GetSize(m_pBackBuffer, &width, &height);
        AVG_TRACE(Logger::BLTS, "Swap Blit: " << 
                rc.tl.x << "x" << rc.tl.y << ", width: " << 
                rc.Width() << ", height: " << rc.Height());

    }
    if (!m_IsFullscreen) {
        pLayerSurf->Flip(pLayerSurf, 0, DFBSurfaceFlipFlags(DSFLIP_BLIT));
    
        AVG_TRACE(Logger::BLTS, "DFB Surface Flip Blit");
    }
    pLayerSurf->Release(pLayerSurf);
}

ISurface * DFBDisplayEngine::createSurface()
{
    return new DFBSurface;
}

IDirectFBSurface * DFBDisplayEngine::getPrimary()
{
    return m_pBackBuffer;
}

bool DFBDisplayEngine::supportsBpp(int bpp)
{
    if (bpp == 16 || bpp == 24 || bpp == 32) {
        return true;
    }
    return false;
}

bool DFBDisplayEngine::hasRGBOrdering()
{
    return false;
}

void DFBDisplayEngine::showCursor (bool bShow)
{
    DFBResult err;
    if (bShow) {    
        err = m_pDFBLayer->SetCursorOpacity(m_pDFBLayer, 255);
    } else {
        err = m_pDFBLayer->SetCursorOpacity(m_pDFBLayer, 0);
    }
    DFBErrorCheck(AVG_ERR_DFB,"DFBDisplayEngine::showCursor",  err);
}

BitmapPtr DFBDisplayEngine::screenshot ()
{
    IDirectFBSurface * pSurface;
    m_pDFBLayer->GetSurface(m_pDFBLayer, &pSurface);
    unsigned char * pBits;
    int Pitch;
    pSurface->Lock(pSurface, DSLF_WRITE, (void **)&pBits, &Pitch);
    PixelFormat pf;
    switch (m_bpp) {
        case 15:
        case 16:
            pf = R5G6B5;
            break;
        case 24:
            pf = R8G8B8;
            break;
        case 32:
            pf = X8R8G8B8;
            break;
        default:
            assert(false);
    }
    return BitmapPtr(new Bitmap(IntPoint(m_Width, m_Height), pf, 
                pBits, Pitch, true));
}

vector<Event *> DFBDisplayEngine::pollEvents()
{
    vector<Event *> Events;
    DFBEvent dfbEvent;
    while(m_pEventBuffer->HasEvent(m_pEventBuffer) == DFB_OK) {
        m_pEventBuffer->GetEvent (m_pEventBuffer, &dfbEvent);
        if (dfbEvent.clazz == DFEC_WINDOW) {
            DFBWindowEvent* pdfbWEvent = &(dfbEvent.window);

            Event * pEvent = createEvent(pdfbWEvent);
            if (pEvent) {
                Events.push_back(pEvent);       
            }
        } else {
            AVG_TRACE(Logger::ERROR, "Unexpected event received.");
        }
    }
        
    return Events;
}


int DFBDisplayEngine::translateModifiers
        (DFBInputDeviceModifierMask DFBModifiers)
{
    int Modifier = 0;
    if (DFBModifiers && DIMM_SHIFT) {
        Modifier |= key::KEYMOD_SHIFT;
    }
    if (DFBModifiers && DIMM_CONTROL) {
        Modifier |= key::KEYMOD_CTRL;
    }
    if (DFBModifiers && DIMM_ALT) {
        Modifier |= key::KEYMOD_ALT;
    }
    if (DFBModifiers && DIMM_ALTGR) {
        Modifier |= key::KEYMOD_RALT;
    }
    if (DFBModifiers && DIMM_META) {
        Modifier |= key::KEYMOD_META;
    }
    return Modifier;    
}

Event * DFBDisplayEngine::createEvent(DFBWindowEvent* pdfbwEvent)
{
    Event * pEvent = 0;
    switch(pdfbwEvent->type) {
        case DWET_KEYDOWN:
        case DWET_KEYUP:
            {
                // TODO: This only works for normal keys... 
                //       Function key mapping etc.
                //       is screwed up badly.
                string KeyString;
                KeyString[0] = char(pdfbwEvent->key_symbol);
                Event::Type Type;
                if (pdfbwEvent->type == DWET_KEYDOWN) {
                    Type = Event::KEYDOWN;
                } else {
                    Type = Event::KEYUP;
                }
                pEvent = new KeyEvent(Type,
                        pdfbwEvent->key_code, pdfbwEvent->key_symbol, KeyString, 
                        translateModifiers(pdfbwEvent->modifiers));
            }
            break;
        case DWET_BUTTONDOWN:
        case DWET_BUTTONUP:
        case DWET_MOTION:
            int Button; 
            Button = 0;
            switch (pdfbwEvent->button) {
                case DIBI_LEFT:
                    Button = MouseEvent::LEFT_BUTTON;
                    break;
                case DIBI_MIDDLE:
                    Button = MouseEvent::MIDDLE_BUTTON;
                    break;
                case DIBI_RIGHT:
                    Button = MouseEvent::RIGHT_BUTTON;
                    break;
                default:
                    break;
            }
            Event::Type Type;
            switch (pdfbwEvent->type) {
                case DWET_BUTTONDOWN:
                    Type = Event::MOUSEBUTTONDOWN;
                    break;
                case DWET_BUTTONUP:
                    Type = Event::MOUSEBUTTONUP;
                    break;
                case DWET_MOTION:
                    Type = Event::MOUSEMOTION;
                    break;
                default:
                    fatalError("Unknown event type in DFBDisplayEngine::createEvent.");
                    Type = Event::QUIT;
                    break;
            }
            pEvent = new MouseEvent(Type,
                    (pdfbwEvent->buttons & DIBM_LEFT)!=0, 
                    (pdfbwEvent->buttons & DIBM_MIDDLE)!=0,
                    (pdfbwEvent->buttons & DIBM_RIGHT)!=0,
                    pdfbwEvent->cx, pdfbwEvent->cy, Button);
            break;
        default:
            break;

    }
    return pEvent;
}

int DFBDisplayEngine::getWidth()
{
    return m_Width;
}

int DFBDisplayEngine::getHeight()
{
    return m_Height;
}

int DFBDisplayEngine::getBPP()
{
    return m_bpp;
}

void DFBDisplayEngine::DFBErrorCheck(int avgcode, string where, 
        DFBResult dfbcode) 
{
    if (dfbcode) {
        throw Exception(avgcode, 
                string("DFB error in ") + where + ": " 
                        + DirectFBErrorString(dfbcode));
    }
}

}

