//
// $Id$
//

#include "AVGDFBDisplayEngine.h"
#include "AVGException.h"
#include "AVGRegion.h"
#include "AVGPlayer.h"
#include "AVGNode.h"
#include "AVGLogger.h"
#include "AVGFramerateManager.h"
#include "AVGDFBFontManager.h"
#include "AVGEvent.h"
#include "AVGMouseEvent.h"
#include "AVGKeyEvent.h"
#include "AVGWindowEvent.h"

#include <paintlib/plbitmap.h>
#include <paintlib/pldirectfbbmp.h>
#include <paintlib/plrect.h>

#include <directfb.h>
#include <signal.h>

#include <iostream>
#include <sstream>
#include <unistd.h>

using namespace std;

AVGDFBDisplayEngine::AVGDFBDisplayEngine()
    : m_pBackBuffer(0),
      m_pDirectFB(0),
      m_pFontManager(0)
{
}

AVGDFBDisplayEngine::~AVGDFBDisplayEngine()
{
    if (m_pDirectFB) {
        teardown();
    }
}

void AVGDFBDisplayEngine::dumpSurface (IDirectFBSurface * pSurf, const string & name)
{
    int w, h;
    AVG_TRACE(AVGPlayer::DEBUG_BLTS, "Surface: " << name);

    pSurf->GetSize(pSurf, &w, &h);
    AVG_TRACE(AVGPlayer::DEBUG_BLTS, "  Size: " << w << "x" << h);

    DFBRectangle rect;
    pSurf->GetVisibleRectangle(pSurf, &rect);
    AVG_TRACE(AVGPlayer::DEBUG_BLTS, "  VisibleRect: x: " << rect.x << 
            ", y: " << rect.y << ", w: " << rect.w << ", h: " << rect.h);

    DFBSurfaceCapabilities caps;
    pSurf->GetCapabilities(pSurf, &caps);
    AVG_TRACE(AVGPlayer::DEBUG_BLTS, "  Caps: " << std::hex << caps);

    DFBSurfacePixelFormat fmt;
    pSurf->GetPixelFormat(pSurf, &fmt);
    AVG_TRACE(AVGPlayer::DEBUG_BLTS, "  PixelFormat: " << fmt << std::dec);
}


void AVGDFBDisplayEngine::init(int width, int height, bool isFullscreen, int bpp)
{
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
    DFBErrorCheck(AVG_ERR_VIDEO_INIT_FAILED, "AVGDFBDisplayEngine::init", err);
    dumpSurface (pLayerSurf, "Layer surface");

    dumpSurface (m_pBackBuffer, "Backbuffer surface");

    m_pFontManager = new AVGDFBFontManager(m_pDirectFB);
    pLayerSurf->Release(pLayerSurf);
    
    // SDL sets up a signal handler we really don't want.
    if (!m_IsFullscreen) {
        signal(SIGSEGV, SIG_DFL);
    }
}

void AVGDFBDisplayEngine::initDFB(int width, int height, bool isFullscreen, int bpp)
{
    // Init DFB system
    char ** argv = new (char *)[7];
    int argc = 3;
    argv[0] = strdup ("bogus_appname");
    argv[1] = strdup("--dfb:no-banner");
    argv[2] = strdup("--dfb:quiet");
    
    if (isFullscreen && geteuid() != 0) {
        isFullscreen = false;
        AVG_TRACE(IAVGPlayer::DEBUG_PROFILE, "Fullscreen requested but not running as root.");
        AVG_TRACE(IAVGPlayer::DEBUG_PROFILE, "         Falling back to windowed mode.");
    }
    
    if (!isFullscreen) {
        argc = 7;
        char tmp[256];
        sprintf(tmp, "--dfb:mode=%ix%i", width, height);
        argv[3] = strdup (tmp);
        if (bpp == 16) {
            argv[4] = strdup ("--dfb:pixelformat=RGB16");
        } else {
            argv[4] = strdup ("--dfb:pixelformat=RGB24");
        }
        argv[5] = strdup ("--dfb:force-windowed");
        argv[6] = strdup ("--dfb:system=SDL");
    }

    DFBResult err;
    err = DirectFBInit (&argc, &argv);
    DFBErrorCheck(AVG_ERR_VIDEO_INIT_FAILED, "AVGDFBDisplayEngine::initDFB - DirectFBInit", err);
    err = DirectFBCreate (&m_pDirectFB);
    DFBErrorCheck(AVG_ERR_VIDEO_INIT_FAILED, "AVGDFBDisplayEngine::initDFB - DirectFBCreate", err);
    
    m_IsFullscreen = isFullscreen;
    m_bpp = bpp;
    PLDirectFBBmp::SetDirectFB(m_pDirectFB);
}

void AVGDFBDisplayEngine::initLayer(int width, int height)
{
    DFBResult err;
    IDirectFBDisplayLayer * pDFBLayer;
    err = m_pDirectFB->GetDisplayLayer(m_pDirectFB, DLID_PRIMARY, &m_pDFBLayer);
    DFBErrorCheck(AVG_ERR_DFB, "AVGDFBDisplayEngine::init", err);
    DFBDisplayLayerDescription LayerDesc;
    err = m_pDFBLayer->GetDescription(m_pDFBLayer, &LayerDesc);
    DFBErrorCheck(AVG_ERR_DFB, "AVGDFBDisplayEngine::init", err);
    PLASSERT (int(LayerDesc.type) && int(DLTF_GRAPHICS) == int(DLTF_GRAPHICS));
    
    DFBDisplayLayerConfig LayerConfig;
    err = m_pDFBLayer->GetConfiguration(m_pDFBLayer, &LayerConfig);
    DFBErrorCheck(AVG_ERR_DFB, "AVGDFBDisplayEngine::init", err);
    m_Width = LayerConfig.width;
    m_Height = LayerConfig.height;
    if (width != m_Width || height != m_Height) {
        cerr << "Warning: avg file expects screen dimensions of " << 
            width << "x" << height << "." << endl;
        cerr << "         Current resolution is " << m_Width << "x" << m_Height << endl;
        cerr << "         To avoid this, change dfb configuration." << endl;
    }

    err = m_pDFBLayer->SetCooperativeLevel(m_pDFBLayer, 
            DFBDisplayLayerCooperativeLevel(DLSCL_ADMINISTRATIVE));
    DFBErrorCheck(AVG_ERR_DFB, "AVGDFBDisplayEngine::init", err);
 
    if (m_IsFullscreen) {
        LayerConfig.flags = DLCONF_BUFFERMODE;
        LayerConfig.buffermode = DLBM_FRONTONLY;  
        err = m_pDFBLayer->SetConfiguration(m_pDFBLayer, &LayerConfig);
        DFBErrorCheck(AVG_ERR_DFB, "AVGDFBDisplayEngine::init", err);

    }
    err = m_pDFBLayer->EnableCursor(m_pDFBLayer, true);
    DFBErrorCheck(AVG_ERR_DFB, "AVGDFBDisplayEngine::init", err);
    err = m_pDFBLayer->SetCursorOpacity(m_pDFBLayer, 0);
    DFBErrorCheck(AVG_ERR_DFB,"AVGDFBDisplayEngine::init",  err);
}


void AVGDFBDisplayEngine::initInput() {
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
    DFBErrorCheck(AVG_ERR_DFB, "AVGDFBDisplayEngine::initInput", err);

    err = m_pDFBWindow->CreateEventBuffer (m_pDFBWindow, &m_pEventBuffer);
    DFBErrorCheck (AVG_ERR_DFB, "AVGDFBDisplayEngine::initInput", err);
    err = m_pDFBWindow->EnableEvents (m_pDFBWindow, DWET_ALL);
    DFBErrorCheck (AVG_ERR_DFB, "AVGDFBDisplayEngine::initInput", err);

    err = m_pDFBWindow->GrabKeyboard(m_pDFBWindow);
    DFBErrorCheck (AVG_ERR_DFB, "AVGDFBDisplayEngine::initInput", err);
    err = m_pDFBWindow->GrabPointer(m_pDFBWindow);
    DFBErrorCheck (AVG_ERR_DFB, "AVGDFBDisplayEngine::initInput", err);
}

void AVGDFBDisplayEngine::initBackbuffer()
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
    DFBErrorCheck (AVG_ERR_DFB, "AVGDFBDisplayEngine::initBackbuffer", err);
}

void AVGDFBDisplayEngine::teardown()
{
    delete m_pFontManager;
    m_pFontManager = 0;

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

void AVGDFBDisplayEngine::render(AVGNode * pRootNode, 
        AVGFramerateManager * pFramerateManager, bool bRenderEverything)
{
    pRootNode->prepareRender(0, pRootNode->getAbsViewport());
    AVGRegion UpdateRegion;
    if (bRenderEverything) {
        PLRect rc(0,0, m_Width, m_Height);
        UpdateRegion.addRect(rc);
    } else {
        pRootNode->getDirtyRegion(UpdateRegion);
    }
//    UpdateRegion.dump();
    for (int i = 0; i<UpdateRegion.getNumRects(); i++) {
        const PLRect & rc = UpdateRegion.getRect(i);
        setDirtyRect(rc);
        setClipRect();
        clear();
        pRootNode->maybeRender(rc);
    }
    pFramerateManager->FrameWait();
    swapBuffers(UpdateRegion);
    pFramerateManager->CheckJitter();
}

void AVGDFBDisplayEngine::setClipRect()
{
    setClipRect(PLRect(0, 0, m_Width, m_Height));
}

bool AVGDFBDisplayEngine::setClipRect(const PLRect& rc)
{
    m_ClipRect = rc;
    m_ClipRect.Intersect(m_DirtyRect);
    if (m_ClipRect.Width() > 0 && m_ClipRect.Height() > 0) {
        DFBRegion Region;
        Region.x1 = m_ClipRect.tl.x;
        Region.y1 = m_ClipRect.tl.y;
        Region.x2 = m_ClipRect.br.x-1;
        Region.y2 = m_ClipRect.br.y-1;
        m_pBackBuffer->SetClip(m_pBackBuffer, &Region);
        AVG_TRACE(AVGPlayer::DEBUG_BLTS, "Clip set to " << 
                m_ClipRect.tl.x << "x" << m_ClipRect.tl.y << 
                ", width: " << m_ClipRect.Width() << ", height: " << 
                m_ClipRect.Height());
        return true;
    } else {
        return false;
    }
}

const PLRect& AVGDFBDisplayEngine::getClipRect() {
    return m_ClipRect;
}

void AVGDFBDisplayEngine::blt32(PLBmp * pBmp, const PLRect* pSrcRect, 
        const PLPoint& pos, double opacity)
{
    PLDirectFBBmp * pDFBBmp = dynamic_cast<PLDirectFBBmp *>(pBmp);
    PLASSERT(pDFBBmp); // createSurface() should have been used to create 
                       // the bitmap.
    IDirectFBSurface * pSurf = pDFBBmp->GetSurface();
    blt32(pSurf, pSrcRect, pos, opacity, pBmp->HasAlpha());
}

void AVGDFBDisplayEngine::blta8(PLBmp * pBmp, const PLRect* pSrcRect,
        const PLPoint& pos, double opacity, const PLPixel32& color)
{
    m_pBackBuffer->SetColor(m_pBackBuffer, color.GetR(), color.GetG(), color.GetB(),
            __u8(opacity*256));

    DFBSurfaceBlittingFlags BltFlags;
    BltFlags = DFBSurfaceBlittingFlags(DSBLIT_BLEND_ALPHACHANNEL | DSBLIT_COLORIZE);
    m_pBackBuffer->SetBlittingFlags(m_pBackBuffer, BltFlags);

    blt(dynamic_cast<PLDirectFBBmp*>(pBmp)->GetSurface(), pSrcRect, pos);
}

void AVGDFBDisplayEngine::blt32(IDirectFBSurface * pSrc, const PLRect* pSrcRect, 
        const PLPoint& pos, double opacity, bool bAlpha)
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
//    dumpSurface (pSurf, "pDFBBmp");
//    dumpSurface (m_pBackBuffer, "m_pBackBuffer");
    blt(pSrc, pSrcRect, pos);
}
 
// Assumes blit flags and colors are already set.
void AVGDFBDisplayEngine::blt(IDirectFBSurface * pSrc, const PLRect* pSrcRect, 
        const PLPoint& pos)
{
    int width;
    int height;
    pSrc->GetSize(pSrc, &width, &height);

    DFBRectangle * pDFBRect = 0;
    DFBRectangle DFBRect;
    if (pSrcRect) {
        pDFBRect = &DFBRect;
        DFBRect.x = pSrcRect->tl.x;
        DFBRect.y = pSrcRect->tl.y;
        DFBRect.w = pSrcRect->Width();
        DFBRect.h = pSrcRect->Height();
    }
    if (m_ClipRect.br.x < pos.x || m_ClipRect.br.y < pos.y) {
        return;
    }
    AVG_TRACE(AVGPlayer::DEBUG_BLTS, "Blit: " << pos.x << "x" << pos.y << 
            ", width:" << width << ", height: " << height);

    DFBResult err = m_pBackBuffer->Blit(m_pBackBuffer, pSrc, pDFBRect, 
            pos.x, pos.y);
        
    DFBErrorCheck(AVG_ERR_VIDEO_GENERAL, "AVGDFBDisplayEngine::blt", err);
}

void AVGDFBDisplayEngine::clear()
{
    DFBResult err;
    m_pBackBuffer->SetDrawingFlags(m_pBackBuffer, DSDRAW_NOFX);
    m_pBackBuffer->SetColor(m_pBackBuffer, 0x0, 0x00, 0x00, 0xff);
    AVG_TRACE(AVGPlayer::DEBUG_BLTS, "Clear rect: " << m_DirtyRect.tl.x << "x" << 
            m_DirtyRect.tl.y << ", width: " << m_DirtyRect.Width() << 
            ", height: " << m_DirtyRect.Height());
    
    err = m_pBackBuffer->FillRectangle(m_pBackBuffer, 
            m_DirtyRect.tl.x, m_DirtyRect.tl.y, 
            m_DirtyRect.Width(), m_DirtyRect.Height());
    DFBErrorCheck(AVG_ERR_VIDEO_GENERAL, "AVGDFBDisplayEngine::clear", err);
}

void AVGDFBDisplayEngine::setDirtyRect(const PLRect& rc) 
{
    m_DirtyRect = rc;
    
    AVG_TRACE(AVGPlayer::DEBUG_BLTS, "Dirty rect: " << m_DirtyRect.tl.x << "x" << 
            m_DirtyRect.tl.y << ", width: " << m_DirtyRect.Width() << 
            ", height: " << m_DirtyRect.Height());
}

void AVGDFBDisplayEngine::swapBuffers(const AVGRegion & UpdateRegion)
{
    DFBResult err;
    IDirectFBSurface * pLayerSurf;
    err = m_pDFBLayer->GetSurface(m_pDFBLayer, &pLayerSurf);
    DFBErrorCheck(AVG_ERR_VIDEO_GENERAL, "AVGDFBDisplayEngine::swapBuffers", err);
    pLayerSurf->SetBlittingFlags(pLayerSurf, DSBLIT_NOFX);
//    m_pDirectFB->WaitForSync(m_pDirectFB);
    for (int i = 0; i<UpdateRegion.getNumRects(); i++) {
        const PLRect & rc = UpdateRegion.getRect(i);
        DFBRectangle DFBRect;
        DFBRect.x = rc.tl.x;
        DFBRect.y = rc.tl.y;
        DFBRect.w = rc.Width();
        DFBRect.h = rc.Height();
        err = pLayerSurf->Blit(pLayerSurf, m_pBackBuffer, &DFBRect, rc.tl.x, rc.tl.y);
        DFBErrorCheck(AVG_ERR_VIDEO_GENERAL, "AVGDFBDisplayEngine::swapBuffers", err);

        int width;
        int height;
        m_pBackBuffer->GetSize(m_pBackBuffer, &width, &height);
        AVG_TRACE(AVGPlayer::DEBUG_BLTS, "Swap Blit: " << 
                rc.tl.x << "x" << rc.tl.y << ", width: " << 
                rc.Width() << ", height: " << rc.Height());

    }
    if (!m_IsFullscreen) {
        pLayerSurf->Flip(pLayerSurf, 0, DFBSurfaceFlipFlags(DSFLIP_BLIT));
    
        AVG_TRACE(AVGPlayer::DEBUG_BLTS, "DFB Surface Flip Blit");
    }
    pLayerSurf->Release(pLayerSurf);
}

PLBmp * AVGDFBDisplayEngine::createSurface()
{
    return new PLDirectFBBmp;
}

IDirectFB * AVGDFBDisplayEngine::getDFB()
{
    return m_pDirectFB;
}

IDirectFBSurface * AVGDFBDisplayEngine::getPrimary()
{
    return m_pBackBuffer;
}

AVGFontManager * AVGDFBDisplayEngine::getFontManager()
{
    return m_pFontManager;
}

vector<AVGEvent *> AVGDFBDisplayEngine::pollEvents()
{
    vector<AVGEvent *> Events;
    DFBEvent dfbEvent;
    while(m_pEventBuffer->HasEvent(m_pEventBuffer) == DFB_OK) {
        m_pEventBuffer->GetEvent (m_pEventBuffer, &dfbEvent);
        if (dfbEvent.clazz == DFEC_WINDOW) {
            DFBWindowEvent* pdfbWEvent = &(dfbEvent.window);

            AVGEvent * pAVGEvent = createEvent(pdfbWEvent);
            if (pAVGEvent) {
                Events.push_back(pAVGEvent);       
            }
        } else {
            AVG_TRACE(IAVGPlayer::DEBUG_ERROR, "Unexpected event received.");
        }
    }
        
    return Events;
}

AVGEvent * AVGDFBDisplayEngine::createEvent(const char * pTypeName)
{
    nsresult rv;
    nsCOMPtr<IAVGEvent> pXPEvent = do_CreateInstance((string("@c-base.org/")+pTypeName+";1").c_str(), &rv);
    PLASSERT(!NS_FAILED(rv));
    NS_IF_ADDREF((IAVGEvent*)pXPEvent);
    return dynamic_cast<AVGEvent*>((IAVGEvent*)pXPEvent);
}

int AVGDFBDisplayEngine::translateModifiers(DFBInputDeviceModifierMask DFBModifiers)
{
    int AVGModifier = 0;
    if (DFBModifiers && DIMM_SHIFT) {
        AVGModifier |= IAVGKeyEvent::KEYMOD_SHIFT;
    }
    if (DFBModifiers && DIMM_CONTROL) {
        AVGModifier |= IAVGKeyEvent::KEYMOD_CTRL;
    }
    if (DFBModifiers && DIMM_ALT) {
        AVGModifier |= IAVGKeyEvent::KEYMOD_ALT;
    }
    if (DFBModifiers && DIMM_ALTGR) {
        AVGModifier |= IAVGKeyEvent::KEYMOD_RALT;
    }
    if (DFBModifiers && DIMM_META) {
        AVGModifier |= IAVGKeyEvent::KEYMOD_META;
    }
    return AVGModifier;    
}

AVGEvent * AVGDFBDisplayEngine::createEvent(DFBWindowEvent* pdfbwEvent)
{
    AVGEvent * pAVGEvent = 0;
    switch(pdfbwEvent->type) {
        case DWET_KEYDOWN:
        case DWET_KEYUP:
            {
                // TODO: This only works for normal keys... Function key mapping etc.
                //       is bound to be screwed up badly.
                pAVGEvent = createEvent("avgkeyevent");
                string KeyString;
                KeyString[0] = char(pdfbwEvent->key_symbol);
                int Type;
                if (pdfbwEvent->type == DWET_KEYDOWN) {
                    Type = IAVGEvent::KEY_DOWN;
                } else {
                    Type = IAVGEvent::KEY_UP;
                }
                dynamic_cast<AVGKeyEvent *>(pAVGEvent)->init(Type,
                        pdfbwEvent->key_code, pdfbwEvent->key_symbol, KeyString, 
                        translateModifiers(pdfbwEvent->modifiers));
            }
            break;
        case DWET_BUTTONDOWN:
        case DWET_BUTTONUP:
        case DWET_MOTION:
            pAVGEvent = createEvent("avgmouseevent");
            int Button;
            switch (pdfbwEvent->button) {
                case DIBI_LEFT:
                    Button = IAVGMouseEvent::LEFT_BUTTON;
                    break;
                case DIBI_MIDDLE:
                    Button = IAVGMouseEvent::MIDDLE_BUTTON;
                    break;
                case DIBI_RIGHT:
                    Button = IAVGMouseEvent::RIGHT_BUTTON;
                    break;
            }
            int Type;
            switch (pdfbwEvent->type) {
                case DWET_BUTTONDOWN:
                    Type = IAVGEvent::MOUSE_BUTTON_DOWN;
                    break;
                case DWET_BUTTONUP:
                    Type = IAVGEvent::MOUSE_BUTTON_UP;
                    break;
                case DWET_MOTION:
                    Type = IAVGEvent::MOUSE_MOTION;
                    break;
            }
            dynamic_cast<AVGMouseEvent *>(pAVGEvent)->init(Type,
                    (pdfbwEvent->buttons & DIBM_LEFT)!=0, 
                    (pdfbwEvent->buttons & DIBM_MIDDLE)!=0,
                    (pdfbwEvent->buttons & DIBM_RIGHT)!=0,
                    pdfbwEvent->cx, pdfbwEvent->cy, Button);
            break;
    }
    return pAVGEvent;
}

int AVGDFBDisplayEngine::getWidth()
{
    return m_Width;
}

int AVGDFBDisplayEngine::getHeight()
{
    return m_Height;
}

int AVGDFBDisplayEngine::getBPP()
{
    return m_bpp;
}

void AVGDFBDisplayEngine::DFBErrorCheck(int avgcode, string where, DFBResult dfbcode) {
    if (dfbcode) {
        throw AVGException(avgcode, 
                string("DFB error in ") + where + ": " + DirectFBErrorString(dfbcode));
    }
}
