//
// $Id$
// 

#include "AVGVideo.h"
#include "AVGDFBDisplayEngine.h"
#include "AVGException.h"

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

NS_IMETHODIMP AVGVideo::GetNumFrames(float *_retval)
{
    *_retval = mpeg3_video_frames(m_pMPEG, 0);
    return NS_OK;
}

NS_IMETHODIMP AVGVideo::GetCurFrame(float *_retval)
{
    return NS_OK;
}

NS_IMETHODIMP AVGVideo::SeekToFrame(float num)
{
    return NS_OK;
}

NS_IMETHODIMP AVGVideo::GetFPS(PRInt32 *_retval)
{
    return NS_OK;
}

void AVGVideo::init (const std::string& id, int x, int y, int z, 
       int width, int height, double opacity, const std::string& filename,
       bool bLoop, bool bOverlay, 
       AVGDFBDisplayEngine * pEngine, AVGContainer * pParent)
{
    AVGNode::init(id, pEngine, pParent);
    
    m_Filename = filename;
    cerr << "Video: opening " << m_Filename << endl;
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
                if (getEffectiveOpacity() < 0.999) {
                    readFrame();
                    getEngine()->render(m_pBmp, getAbsViewport().tl, getEffectiveOpacity());
                } else {
                    // No alpha blending: render frame to backbuffer directly.
                    renderToBackbuffer();
                }
            }
            break;
        case Paused:
            if (!m_bFrameAvailable) {
                readFrame();
            }
            getEngine()->render(m_pBmp, getAbsViewport().tl, getEffectiveOpacity());
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

    if (mpeg3_frame_rate(m_pMPEG, 0) != 30) {
        cerr << "Warning: video " << m_Filename << " has framerate " 
            << mpeg3_frame_rate(m_pMPEG, 0) << ". Should be 30." << endl;
    }
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
    // libmpeg3 forgets to turn mmx off, killing floating point operations.
    // TODO: Add an ifdef for intel here.
    __asm__ __volatile__ ("emms");  
    m_bFrameAvailable = true;
    advancePlayback();
}

void AVGVideo::renderToBackbuffer()
{
    PLRect vpt = getAbsViewport();
    // Calc row ptr array.
    IDirectFBSurface * pSurface = getEngine()->getPrimary();
    PLBYTE * pBits;
    int Pitch;
    DFBResult err = pSurface->Lock(pSurface, DFBSurfaceLockFlags(DSLF_WRITE), 
            (void **)&pBits, &Pitch);
    getEngine()->DFBErrorCheck(AVG_ERR_DFB, "AVGVideo::renderToBackbuffer", err);
    PLBYTE ** ppRows = new (PLBYTE *)[vpt.Height()];
    for (int y=vpt.tl.y; y<vpt.br.y; y++) {
        ppRows[y-vpt.tl.y] = pBits+Pitch*y+2*vpt.tl.x;
    }
    mpeg3_read_frame(m_pMPEG, ppRows, 0, 0,
            m_Width-1, m_Height-1,
            vpt.Width(), vpt.Height(), 
            MPEG3_RGB565, 0);

    delete[] ppRows;
    pSurface->Unlock(pSurface);
    
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
    if (m_CurFrame >= mpeg3_video_frames(m_pMPEG, 0)-5) {
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


