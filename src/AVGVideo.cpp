//
// $Id$
// 

#include "AVGVideo.h"
#include "AVGDFBDisplayEngine.h"
#include "AVGException.h"
#include "AVGPlayer.h"
#include "AVGLogger.h"
#include "AVGContainer.h"

#include <paintlib/plbitmap.h>
#include <paintlib/pldirectfbbmp.h>
#include <paintlib/planybmp.h>
#include <paintlib/Filter/plfilterfill.h>

#include <nsMemory.h>
#include <xpcom/nsComponentManagerUtils.h>

#include <iostream>
#include <sstream>
#include <unistd.h>

using namespace std;

NS_IMPL_ISUPPORTS2_CI(AVGVideo, IAVGNode, IAVGVideo);


AVGVideo * AVGVideo::create()
{
    return createNode<AVGVideo>("@c-base.org/avgvideo;1");
}       

AVGVideo::AVGVideo ()
    : m_pMPEG(0),
      m_State(Unloaded)
{
    NS_INIT_ISUPPORTS();
    m_bFrameAvailable = false;
}

AVGVideo::~AVGVideo ()
{
    if (m_pMPEG) {
        mpeg3_close(m_pMPEG);
    }
    if (m_pBmp) {
        delete m_pBmp;
    }
}

NS_IMETHODIMP 
AVGVideo::GetType(PRInt32 *_retval)
{
    *_retval = NT_VIDEO;
    return NS_OK;
}

NS_IMETHODIMP 
AVGVideo::Play()
{
    changeState(Playing);
    return NS_OK;
}

NS_IMETHODIMP AVGVideo::Stop()
{
    changeState(Unloaded);
    return NS_OK;
}

NS_IMETHODIMP AVGVideo::Pause()
{
    changeState(Paused);
    return NS_OK;
}

NS_IMETHODIMP AVGVideo::GetNumFrames(int *_retval)
{
    if (m_State != Unloaded) {
        *_retval = mpeg3_video_frames(m_pMPEG, 0);
    } else {
        AVG_TRACE(IAVGPlayer::DEBUG_ERROR,
               "Error in AVGVideo::GetNumFrames: Video not loaded.");
        *_retval = -1;
    }
    return NS_OK;
}

NS_IMETHODIMP AVGVideo::GetCurFrame(int *_retval)
{
    if (m_State != Unloaded) {
        *_retval = mpeg3_get_frame(m_pMPEG, 0);
    } else {
        AVG_TRACE(IAVGPlayer::DEBUG_ERROR, "Error in AVGVideo::GetCurFrame: Video not loaded.");
        *_retval = -1;
    }
    return NS_OK;
}

NS_IMETHODIMP AVGVideo::SeekToFrame(int num)
{
    // TODO: Figure out why/when libmpeg3 has problems seeking.
    if (m_State != Unloaded) {
        mpeg3_set_frame(m_pMPEG, num, 0);
    } else {
        AVG_TRACE(IAVGPlayer::DEBUG_ERROR, "Error in AVGVideo::SeekToFrame: Video not loaded.");
    }
    return NS_OK;
}

NS_IMETHODIMP AVGVideo::GetFPS(PRInt32 *_retval)
{
    if (m_State != Unloaded) {
        *_retval = (int)mpeg3_frame_rate(m_pMPEG, 0);
    } else {
        *_retval = -1;
    }
    return NS_OK;
}

void AVGVideo::init (const std::string& id, int x, int y, int z, 
       int width, int height, double opacity, const std::string& filename,
       bool bLoop, bool bOverlay, 
       IAVGDisplayEngine * pEngine, AVGContainer * pParent, AVGPlayer * pPlayer)
{
    AVGNode::init(id, pEngine, pParent, pPlayer);
    
    m_Filename = filename;
    AVG_TRACE(AVGPlayer::DEBUG_MEMORY, "Video: opening " << m_Filename << endl);
    open (&m_Width, &m_Height);
    m_bLoop = bLoop;
    m_bOverlay = bOverlay;
    if (m_bOverlay) {
        initOverlay();
    }
    
    initVisible(x, y, z, width, height, opacity); 
    changeState (Paused);
}

void AVGVideo::prepareRender (int time, const PLRect& parent)
{
    AVGNode::prepareRender(time, parent);
    if (m_State == Playing) {
        invalidate();
    }
}

void AVGVideo::render (const PLRect& Rect)
{
    switch(m_State) 
    {
        case Playing:
            if (m_bOverlay) {
                renderToOverlay();    
            } else {
                if (getEffectiveOpacity() < 0.001) {
                    return;
                }
                PLRect relVpt = getRelViewport();
                PLRect absVpt = getParent()->getAbsViewport();   
                if (getEffectiveOpacity() > 0.999 && 
                    dynamic_cast<AVGDFBDisplayEngine*>(getEngine()) &&
                    relVpt.tl.x >= 0 && relVpt.tl.y >= 0 && 
                    absVpt.Width() > relVpt.br.x && absVpt.Height() > relVpt.br.y)
                {
                    // No alpha blending: render frame to backbuffer directly.
                    // (DirectFB only).
                    renderToBackbuffer();
                } else {
                    readFrame();
                    getEngine()->blt32(m_pBmp, 0, getAbsViewport().tl, getEffectiveOpacity());
                }
            }
            break;
        case Paused:
            if (!m_bFrameAvailable) {
                readFrame();
            }
            getEngine()->blt32(m_pBmp, 0, getAbsViewport().tl, getEffectiveOpacity());
            break;
        case Unloaded:
            break;
    }
}

string AVGVideo::getTypeStr ()
{
    return "AVGVideo";
}

void AVGVideo::changeState(VideoState NewState)
{
    if (m_State == NewState) {
        return;
    }
    if (m_State == Unloaded) {
        if (!m_pMPEG) {
            open (&m_Width, &m_Height);
        }

        m_pBmp = getEngine()->createSurface();
        PLRect vpt = getRelViewport();
        
        // libmpeg3 wants the bitmap to be larger than nessesary :-(.
        m_pBmp->Create(vpt.Width(), vpt.Height()+1, 32, false, false);
        m_bFrameAvailable = false;
    }
    if (NewState == Unloaded) {
        mpeg3_close(m_pMPEG);
        m_pMPEG = 0;
        delete m_pBmp;
        m_pBmp = 0;
    }
    addDirtyRect(getAbsViewport());
    m_State = NewState;
}

void AVGVideo::open (int* pWidth, int* pHeight)
{
    int ok = mpeg3_check_sig(const_cast<char*>(m_Filename.c_str()));
    if (ok != 1) {
        throw AVGException(AVG_ERR_VIDEO_LOAD_FAILED, 
                m_Filename + " is not a valid mpeg file (check_sig failed).");
    }
    m_pMPEG = mpeg3_open(const_cast<char*>(m_Filename.c_str()));
    if (!m_pMPEG) {
        throw AVGException(AVG_ERR_VIDEO_LOAD_FAILED, 
                m_Filename + " is not a valid mpeg file (open failed).");
    }
    mpeg3_set_mmx(m_pMPEG, 0);


    if (!mpeg3_total_vstreams(m_pMPEG)) {
        throw AVGException(AVG_ERR_VIDEO_LOAD_FAILED, 
                m_Filename + " does not contain any video streams.");
    }

    *pWidth = mpeg3_video_width(m_pMPEG, 0);
    *pHeight = mpeg3_video_height(m_pMPEG, 0);

    m_CurFrame = 0;
}

void dumpBmpLineArray(PLBmp* pBmp)
{
    PLBYTE ** ppLines = pBmp->GetLineArray();
    for (int y = 0; y < pBmp->GetHeight(); y++) {
        cerr << "Line " << y << ": " << hex << (void*)ppLines[y] << dec << endl;
    }
}

void AVGVideo::readFrame()
{
    PLRect vpt = getRelViewport();
    mpeg3_read_frame(m_pMPEG, m_pBmp->GetLineArray(), 0, 0, 
            m_Width-1, m_Height-1, vpt.Width(), vpt.Height(), 
            MPEG3_RGBA8888, 0);
#ifdef i386
    // libmpeg3 forgets to turn mmx off, killing floating point operations.
    __asm__ __volatile__ ("emms");  
#endif    
    m_bFrameAvailable = true;
    advancePlayback();
}

void AVGVideo::renderToBackbuffer()
{
    AVGDFBDisplayEngine* pEngine = dynamic_cast<AVGDFBDisplayEngine*>(getEngine());
    PLRect vpt = getAbsViewport();
    // Calc row ptr array.
    IDirectFBSurface * pSurface = pEngine->getPrimary();
    PLBYTE * pBits;
    int Pitch;
    DFBResult err = pSurface->Lock(pSurface, DFBSurfaceLockFlags(DSLF_WRITE), 
            (void **)&pBits, &Pitch);
    pEngine->DFBErrorCheck(AVG_ERR_DFB, "AVGVideo::renderToBackbuffer", err);
    PLBYTE ** ppRows = new (PLBYTE *)[vpt.Height()];
    int BytesPerPixel;
    int ColorModel;
    if (getEngine()->getBPP() == 16) {
        BytesPerPixel = 2;
        ColorModel = MPEG3_RGB565;
    } else {
        BytesPerPixel = 3;
        ColorModel = MPEG3_RGB888;
    }
    for (int y=vpt.tl.y; y<vpt.br.y; y++) {
        ppRows[y-vpt.tl.y] = pBits+Pitch*y+BytesPerPixel*vpt.tl.x;
    }
    mpeg3_read_frame(m_pMPEG, ppRows, 0, 0,
            m_Width-1, m_Height-1,
            vpt.Width(), vpt.Height(), 
            ColorModel, 0);
#ifdef i386
    // libmpeg3 forgets to turn mmx off, killing floating point operations.
    __asm__ __volatile__ ("emms");  
#endif    

    delete[] ppRows;
    pSurface->Unlock(pSurface);
    m_bFrameAvailable=false;
    advancePlayback();
}

void AVGVideo::initOverlay()
{
//    getEngine()->getOverlaySurface();
}

void AVGVideo::renderToOverlay()
{
}

bool AVGVideo::obscures (const PLRect& Rect, int z)
{
    return (getEffectiveOpacity() > 0.999 &&
            getZ() > z && getAbsViewport().Contains(Rect));

}

void AVGVideo::advancePlayback()
{
    m_CurFrame++;
    if (m_CurFrame >= mpeg3_video_frames(m_pMPEG, 0)) {
        if (m_bLoop) {
            mpeg3_set_frame(m_pMPEG, 0, 0);
            m_CurFrame = 0;
        } else {
            changeState(Paused);
        }
    }
}

string AVGVideo::dump (int indent)
{
    stringstream s;
    s << AVGNode::dump(indent)
        << string(indent+8,  ' ') << "File: " << m_Filename
        << ", audio streams: " << mpeg3_total_astreams(m_pMPEG)
        << ", video streams: " << mpeg3_total_vstreams(m_pMPEG) << "," << endl
        << string(indent+8,  ' ')
        << "video size: " << mpeg3_video_width(m_pMPEG, 0)
        << "x" << mpeg3_video_height(m_pMPEG, 0) << "," << endl
        << string(indent+8, ' ') << "aspect ratio: " << mpeg3_aspect_ratio(m_pMPEG, 0)
        << ", framerate: " << mpeg3_frame_rate(m_pMPEG, 0) 
        << ", frames: " << mpeg3_video_frames(m_pMPEG, 0);

    return s.str();
}

PLPoint AVGVideo::getPreferredMediaSize()
{
    return PLPoint(m_Width, m_Height);
}


