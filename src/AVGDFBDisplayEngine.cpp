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
      m_pDirectFB(0)
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


void AVGDFBDisplayEngine::init(int width, int height, bool isFullscreen)
{
    // Init DFB system
    char ** argv = new (char *)[3];
    int argc = 1;
    argv[0] = strdup ("bogus_appname");
    if (!isFullscreen) {
        argc = 3;
        argv[1] = strdup ("--dfb:force-windowed");
        argv[2] = strdup ("--dfb:system=SDL");
    }
    DFBResult err;
    err = DirectFBInit (&argc, &argv);
    DFBErrorCheck(AVG_ERR_VIDEO_INIT_FAILED, err);
    err = DirectFBCreate (&m_pDirectFB);
    DFBErrorCheck(AVG_ERR_VIDEO_INIT_FAILED, err);
/*    if (isFullscreen) {
        err = m_pDirectFB->SetCooperativeLevel(m_pDirectFB, DFSCL_EXCLUSIVE);
        DFBErrorCheck(AVG_ERR_VIDEO_INIT_FAILED, err);
    }
*/
    m_IsFullscreen = isFullscreen;

    PLDirectFBBmp::SetDirectFB(m_pDirectFB);

    // Init layer
    IDirectFBDisplayLayer * pDFBLayer;
    err = m_pDirectFB->GetDisplayLayer(m_pDirectFB, DLID_PRIMARY, &m_pDFBLayer);
    DFBErrorCheck(AVG_ERR_DFB, err);
    DFBDisplayLayerDescription LayerDesc;
    err = m_pDFBLayer->GetDescription(m_pDFBLayer, &LayerDesc);
    DFBErrorCheck(AVG_ERR_DFB, err);
    PLASSERT (int(LayerDesc.type) && int(DLTF_GRAPHICS) == int(DLTF_GRAPHICS));
    DFBDisplayLayerConfig LayerConfig;
    err = m_pDFBLayer->GetConfiguration(m_pDFBLayer, &LayerConfig);
    DFBErrorCheck(AVG_ERR_DFB, err);
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
    DFBErrorCheck(AVG_ERR_DFB, err);

    if (m_IsFullscreen) {
        err = m_pDFBLayer->SetCooperativeLevel(m_pDFBLayer, 
                DFBDisplayLayerCooperativeLevel(DLSCL_ADMINISTRATIVE));
        DFBErrorCheck(AVG_ERR_DFB, err);
        err = m_pDFBLayer->EnableCursor(m_pDFBLayer, true);
        DFBErrorCheck(AVG_ERR_DFB, err);
        err = m_pDFBLayer->SetCursorOpacity(m_pDFBLayer, 0);
        DFBErrorCheck(AVG_ERR_DFB, err);
   }
   
    LayerConfig.flags = DLCONF_BUFFERMODE;
    LayerConfig.buffermode = DLBM_FRONTONLY;  
//    LayerConfig.buffermode = DLBM_BACKSYSTEM; // Backbuffer in system memory.
    err = m_pDFBLayer->SetConfiguration(m_pDFBLayer, &LayerConfig);
    DFBErrorCheck(AVG_ERR_DFB, err);

    // Init window
    DFBWindowDescription WinDesc;
    WinDesc.flags = DFBWindowDescriptionFlags(DWDESC_CAPS | DWDESC_WIDTH | 
            DWDESC_HEIGHT | DWDESC_POSX | DWDESC_POSY | DWDESC_SURFACE_CAPS);
    WinDesc.caps = DFBWindowCapabilities(DWCAPS_NONE); // | DWCAPS_DOUBLEBUFFER); 
    WinDesc.width = m_Width;
    WinDesc.height = m_Height;
    WinDesc.posx = 0;
    WinDesc.posy = 0;
    WinDesc.surface_caps = DSCAPS_FLIPPING;
    err = m_pDFBLayer->CreateWindow(m_pDFBLayer, &WinDesc, &m_pDFBWindow);
    DFBErrorCheck(AVG_ERR_DFB, err);
    err = m_pDFBWindow->SetOpacity (m_pDFBWindow, 0xFF);
    DFBErrorCheck(AVG_ERR_DFB, err);

    initInput();

    IDirectFBSurface * pLayerSurf;
    err = m_pDFBLayer->GetSurface(m_pDFBLayer, &pLayerSurf);
    DFBErrorCheck(AVG_ERR_VIDEO_INIT_FAILED, err);
    dumpSurface (pLayerSurf, "Layer surface");

    err = m_pDFBWindow->GetSurface(m_pDFBWindow, &m_pPrimary);
    DFBErrorCheck(AVG_ERR_VIDEO_INIT_FAILED, err);
    dumpSurface (m_pPrimary, "Window surface (m_pPrimary)");
}

void AVGDFBDisplayEngine::initInput() {
    DFBResult err;    
    err = m_pDFBWindow->CreateEventBuffer (m_pDFBWindow, &m_pEventBuffer);
    DFBErrorCheck (AVG_ERR_DFB, err);
    err = m_pDFBWindow->EnableEvents (m_pDFBWindow, DWET_ALL);
    DFBErrorCheck (AVG_ERR_DFB, err);

    err = m_pDFBWindow->GrabKeyboard(m_pDFBWindow);
    DFBErrorCheck (AVG_ERR_DFB, err);
    err = m_pDFBWindow->GrabPointer(m_pDFBWindow);
    DFBErrorCheck (AVG_ERR_DFB, err);
}

void AVGDFBDisplayEngine::teardown()
{
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

void AVGDFBDisplayEngine::setClipRect(const PLRect& rc)
{
    m_ClipRect = rc;
}

void AVGDFBDisplayEngine::setDirtyRect(const PLRect& rc) 
{
    PLRect DirtyRect = rc;
    DirtyRect.Intersect(m_ClipRect);
    m_DirtyRect = DirtyRect;
    DFBRegion Region;
    Region.x1 = DirtyRect.tl.x;
    Region.y1 = DirtyRect.tl.y;
    Region.x2 = DirtyRect.br.x-1;
    Region.y2 = DirtyRect.br.y-1;
    m_pPrimary->SetClip(m_pPrimary, &Region);
}

void AVGDFBDisplayEngine::clear()
{
    DFBResult err;
    m_pPrimary->SetColor(m_pPrimary, 0x0, 0x00, 0x00, 0xff);
//    cerr << "Clear rect: " << m_DirtyRect.tl.x << "x" << m_DirtyRect.tl.y <<", " <<
//            m_DirtyRect.br.x << "x" << m_DirtyRect.br.y << endl;
    err = m_pPrimary->FillRectangle(m_pPrimary, 
            m_DirtyRect.tl.x, m_DirtyRect.tl.y, 
            m_DirtyRect.Width(), m_DirtyRect.Height());
    DFBErrorCheck(AVG_ERR_VIDEO_GENERAL, err);
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
    DFBErrorCheck(AVG_ERR_VIDEO_GENERAL, err);
}

void AVGDFBDisplayEngine::swapBuffers()
{
    DFBResult err;
    if (m_IsFullscreen) {
        IDirectFBSurface * pLayerSurf;
        err = m_pDFBLayer->GetSurface(m_pDFBLayer, &pLayerSurf);
        DFBErrorCheck(AVG_ERR_VIDEO_GENERAL, err);
        err = pLayerSurf->Blit(pLayerSurf, m_pPrimary, 0, 0, 0);
    } else {
        err = m_pPrimary->Flip(m_pPrimary, 0, 
//                DFBSurfaceFlipFlags(DSFLIP_WAITFORSYNC | DSFLIP_BLIT));
                  DFBSurfaceFlipFlags(DSFLIP_BLIT));
    }
    DFBErrorCheck(AVG_ERR_VIDEO_GENERAL, err);
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

void AVGDFBDisplayEngine::DFBErrorCheck(int avgcode, DFBResult dfbcode) {
    if (dfbcode) {
        throw AVGException(avgcode, 
                DirectFBErrorString(dfbcode));
    }
}
