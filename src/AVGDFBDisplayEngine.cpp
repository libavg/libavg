//
// $Id$
//

#include "AVGDFBDisplayEngine.h"
#include "AVGException.h"

#include <paintlib/plbitmap.h>
#include <paintlib/pldirectfbbmp.h>
#include <paintlib/plrect.h>

#include <directfb.h>

#include <iostream>
#include <unistd.h>

using namespace std;

AVGDFBDisplayEngine::AVGDFBDisplayEngine()
    : m_pPrimary(0),
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
    cerr << "Surface: " << name << endl;

    pSurf->GetSize(pSurf, &w, &h);
    cerr << "  Size: " << w << "x" << h << endl;

    DFBRectangle rect;
    pSurf->GetVisibleRectangle(pSurf, &rect);
    cerr << "  VisibleRect: x: " << rect.x << ", y: " << rect.y << 
            ", w: " << rect.w << ", h: " << rect.h << endl;

    cerr.setf(ios::hex);
    DFBSurfaceCapabilities caps;
    pSurf->GetCapabilities(pSurf, &caps);
    cerr << "  Caps: " << caps << endl;

    DFBSurfacePixelFormat fmt;
    pSurf->GetPixelFormat(pSurf, &fmt);
    cerr << "  PixelFormat: " << fmt << endl;
    cerr.setf(ios::dec);
}


void AVGDFBDisplayEngine::init(int width, int height, bool isFullscreen, bool bDebugBlts)
{
    if (m_pDirectFB) {
        teardown();
    }
    // Init DFB system
    char ** argv = new (char *)[5];
    int argc = 1;
    argv[0] = strdup ("bogus_appname");
    if (!isFullscreen) {
        argc = 5;
        argv[1] = strdup ("--dfb:force-windowed");
        argv[2] = strdup ("--dfb:system=SDL");
        char tmp[256];
        sprintf(tmp, "--dfb:mode=%ix%i", width, height);
        argv[3] = strdup (tmp);
        argv[4] = strdup ("--dfb:pixelformat=RGB16");
    }
    DFBResult err;
    err = DirectFBInit (&argc, &argv);
    DFBErrorCheck(AVG_ERR_VIDEO_INIT_FAILED, "AVGDFBDisplayEngine::init", err);
    err = DirectFBCreate (&m_pDirectFB);
    DFBErrorCheck(AVG_ERR_VIDEO_INIT_FAILED, "AVGDFBDisplayEngine::init", err);
/*    if (isFullscreen) {
        err = m_pDirectFB->SetCooperativeLevel(m_pDirectFB, DFSCL_EXCLUSIVE);
        DFBErrorCheck(AVG_ERR_VIDEO_INIT_FAILED, err);
    }
*/
    m_IsFullscreen = isFullscreen;
    m_bDebugBlts = bDebugBlts;

    PLDirectFBBmp::SetDirectFB(m_pDirectFB);

    // Init layer
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
    }

    DFBDisplayLayerCooperativeLevel CoopLevel;
    if (m_IsFullscreen) {
        CoopLevel = DLSCL_EXCLUSIVE;
    } else {
        CoopLevel = DLSCL_ADMINISTRATIVE;
    }
    err = m_pDFBLayer->SetCooperativeLevel(m_pDFBLayer, 
            DFBDisplayLayerCooperativeLevel(CoopLevel));
    DFBErrorCheck(AVG_ERR_DFB, "AVGDFBDisplayEngine::init", err);

    if (m_IsFullscreen) {
        err = m_pDFBLayer->SetCooperativeLevel(m_pDFBLayer, 
                DFBDisplayLayerCooperativeLevel(DLSCL_ADMINISTRATIVE));
        DFBErrorCheck(AVG_ERR_DFB, "AVGDFBDisplayEngine::init", err);
        err = m_pDFBLayer->EnableCursor(m_pDFBLayer, true);
        DFBErrorCheck(AVG_ERR_DFB, "AVGDFBDisplayEngine::init", err);
        err = m_pDFBLayer->SetCursorOpacity(m_pDFBLayer, 0);
        DFBErrorCheck(AVG_ERR_DFB,"AVGDFBDisplayEngine::init",  err);
   }
   
    LayerConfig.flags = DLCONF_BUFFERMODE;
    LayerConfig.buffermode = DLBM_FRONTONLY;  
//    LayerConfig.buffermode = DLBM_BACKSYSTEM; // Backbuffer in system memory.
    err = m_pDFBLayer->SetConfiguration(m_pDFBLayer, &LayerConfig);
    DFBErrorCheck(AVG_ERR_DFB, "AVGDFBDisplayEngine::init", err);

    // Init window
    DFBWindowDescription WinDesc;
    WinDesc.flags = DFBWindowDescriptionFlags(DWDESC_CAPS | DWDESC_WIDTH | DWDESC_PIXELFORMAT |
            DWDESC_HEIGHT | DWDESC_POSX | DWDESC_POSY | DWDESC_SURFACE_CAPS);
    WinDesc.caps = DFBWindowCapabilities(DWCAPS_NONE); // | DWCAPS_DOUBLEBUFFER); 
    WinDesc.width = m_Width;
    WinDesc.height = m_Height;
    WinDesc.posx = 0;
    WinDesc.posy = 0;
    WinDesc.pixelformat=DSPF_RGB16; 
    WinDesc.surface_caps = DFBSurfaceCapabilities(DSCAPS_FLIPPING);
    err = m_pDFBLayer->CreateWindow(m_pDFBLayer, &WinDesc, &m_pDFBWindow);
    DFBErrorCheck(AVG_ERR_DFB, "AVGDFBDisplayEngine::init", err);
    err = m_pDFBWindow->SetOpacity (m_pDFBWindow, 0xFF);
    DFBErrorCheck(AVG_ERR_DFB, "AVGDFBDisplayEngine::init", err);

    initInput();

    IDirectFBSurface * pLayerSurf;
    err = m_pDFBLayer->GetSurface(m_pDFBLayer, &pLayerSurf);
    DFBErrorCheck(AVG_ERR_VIDEO_INIT_FAILED, "AVGDFBDisplayEngine::init", err);
    dumpSurface (pLayerSurf, "Layer surface");

    err = m_pDFBWindow->GetSurface(m_pDFBWindow, &m_pPrimary);
    DFBErrorCheck(AVG_ERR_VIDEO_INIT_FAILED, "AVGDFBDisplayEngine::init", err);
    dumpSurface (m_pPrimary, "Window surface (m_pPrimary)");
    m_pFontManager = new AVGFontManager;
}

void AVGDFBDisplayEngine::initInput() {
    DFBResult err;    
    err = m_pDFBWindow->CreateEventBuffer (m_pDFBWindow, &m_pEventBuffer);
    DFBErrorCheck (AVG_ERR_DFB, "AVGDFBDisplayEngine::initInput", err);
    err = m_pDFBWindow->EnableEvents (m_pDFBWindow, DWET_ALL);
    DFBErrorCheck (AVG_ERR_DFB, "AVGDFBDisplayEngine::initInput", err);

    err = m_pDFBWindow->GrabKeyboard(m_pDFBWindow);
    DFBErrorCheck (AVG_ERR_DFB, "AVGDFBDisplayEngine::initInput", err);
    err = m_pDFBWindow->GrabPointer(m_pDFBWindow);
    DFBErrorCheck (AVG_ERR_DFB, "AVGDFBDisplayEngine::initInput", err);
}

void AVGDFBDisplayEngine::teardown()
{
    delete m_pFontManager;
    m_pFontManager = 0;

    m_pEventBuffer->Release(m_pEventBuffer);
    m_pDFBWindow->Close(m_pDFBWindow);
    m_pDFBWindow->Destroy(m_pDFBWindow);
    m_pDFBWindow->Release(m_pDFBWindow);
    m_pPrimary = 0;
    m_pDFBLayer->Release(m_pDFBLayer);
    m_pDirectFB->Release(m_pDirectFB);
    m_pDirectFB = 0;
}

void AVGDFBDisplayEngine::setClipRect()
{
    m_ClipRect = PLRect(0, 0, m_Width, m_Height);
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
        m_pPrimary->SetClip(m_pPrimary, &Region);
        if (m_bDebugBlts) {
            cerr << "---- Clip set to " << m_ClipRect.tl.x << "x" << 
                    m_ClipRect.tl.y << ", width: " << m_ClipRect.Width() << 
                    ", height: " << m_ClipRect.Height() << endl;
        }
        return true;
    } else {
        return false;
    }
}

const PLRect& AVGDFBDisplayEngine::getClipRect() {
    return m_ClipRect;
}

void AVGDFBDisplayEngine::setDirtyRect(const PLRect& rc) 
{
    m_DirtyRect = rc;
    
    if (m_bDebugBlts) {
        cerr << "Dirty rect: " << m_DirtyRect.tl.x << "x" << 
                    m_DirtyRect.tl.y << ", width: " << m_DirtyRect.Width() << 
                    ", height: " << m_DirtyRect.Height() << endl;
    }
}

void AVGDFBDisplayEngine::clear()
{
    DFBResult err;
    m_pPrimary->SetColor(m_pPrimary, 0x0, 0x00, 0x00, 0xff);
    if (m_bDebugBlts) {
        cerr << "---- Clear rect: " << m_DirtyRect.tl.x << "x" << 
                m_DirtyRect.tl.y << ", width: " << m_DirtyRect.Width() << 
                ", height: " << m_DirtyRect.Height() << endl;
    }        
    err = m_pPrimary->FillRectangle(m_pPrimary, 
            m_DirtyRect.tl.x, m_DirtyRect.tl.y, 
            m_DirtyRect.Width(), m_DirtyRect.Height());
    DFBErrorCheck(AVG_ERR_VIDEO_GENERAL, "AVGDFBDisplayEngine::clear", err);
}

void AVGDFBDisplayEngine::render(PLBmp * pBmp, const PLPoint& pos, double opacity)
{
    PLDirectFBBmp * pDFBBmp = dynamic_cast<PLDirectFBBmp *>(pBmp);
    PLASSERT(pDFBBmp); // createSurface() should have been used to create 
                       // the bitmap.
    IDirectFBSurface * pSurf = pDFBBmp->GetSurface();
    render (pSurf, pos, opacity, pBmp->HasAlpha());
}

void AVGDFBDisplayEngine::render(IDirectFBSurface * pSrc, const PLPoint& pos,   
        double opacity, bool bAlpha)
{
    DFBSurfaceBlittingFlags BltFlags;
    if (bAlpha) {
        BltFlags = DSBLIT_BLEND_ALPHACHANNEL;
    } else {
        BltFlags = DSBLIT_NOFX;
    }
    if (opacity < 0.9999) {
        BltFlags = DFBSurfaceBlittingFlags(BltFlags | DSBLIT_BLEND_COLORALPHA);
        m_pPrimary->SetColor(m_pPrimary, 
                             0xff, 0xff, 0xff, __u8(opacity*256));
    }
    m_pPrimary->SetBlittingFlags(m_pPrimary, BltFlags);
//    dumpSurface (pSurf, "pDFBBmp");
//    dumpSurface (m_pPrimary, "m_pPrimary");
    DFBResult err = m_pPrimary->Blit(m_pPrimary, pSrc, 0, 
            pos.x, pos.y);
    if (m_bDebugBlts) {
        int width;
        int height;
        pSrc->GetSize(pSrc, &width, &height);
        cerr << "---- Blit: " << pos.x << "x" << pos.y << ", width:" << width << 
                ", height: " << height << ", alpha: " << bAlpha << 
                ", opacity: " << opacity << endl;
    }
    DFBErrorCheck(AVG_ERR_VIDEO_GENERAL, "AVGDFBDisplayEngine::render", err);
}

void AVGDFBDisplayEngine::swapBuffers()
{
    DFBResult err;
    if (m_IsFullscreen) {
        IDirectFBSurface * pLayerSurf;
        err = m_pDFBLayer->GetSurface(m_pDFBLayer, &pLayerSurf);
        DFBErrorCheck(AVG_ERR_VIDEO_GENERAL, "AVGDFBDisplayEngine::swapBuffers", err);
        err = pLayerSurf->Blit(pLayerSurf, m_pPrimary, 0, 0, 0);
        if (m_bDebugBlts) {
            int width;
            int height;
            m_pPrimary->GetSize(m_pPrimary, &width, &height);
            if (m_bDebugBlts) {
                cerr << "---- Swap Blit: 0x0, width: " << 
                        width << ", height: " << height << endl;
            }
        }
    } else {
        err = m_pPrimary->Flip(m_pPrimary, 0, 
//                DFBSurfaceFlipFlags(DSFLIP_WAITFORSYNC | DSFLIP_BLIT));
                  DFBSurfaceFlipFlags(DSFLIP_BLIT));
        if (m_bDebugBlts) {
            cerr << "---- Flip" << endl;
        }
    }
    DFBErrorCheck(AVG_ERR_VIDEO_GENERAL, "AVGDFBDisplayEngine::swapBuffers", err);
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
    return m_pPrimary;
}

AVGFontManager * AVGDFBDisplayEngine::getFontManager()
{
    return m_pFontManager;
}


int AVGDFBDisplayEngine::getWidth()
{
    return m_Width;
}

int AVGDFBDisplayEngine::getHeight()
{
    return m_Height;
}

IDirectFBEventBuffer * AVGDFBDisplayEngine::getEventBuffer()
{
    return m_pEventBuffer;
}

void AVGDFBDisplayEngine::DFBErrorCheck(int avgcode, string where, DFBResult dfbcode) {
    if (dfbcode) {
        throw AVGException(avgcode, 
                string("DFB error in ") + where + ": " + DirectFBErrorString(dfbcode));
    }
}
