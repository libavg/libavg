//
// $Id$
// 

#include "AVGVideo.h"
#include "AVGDFBDisplayEngine.h"
#include "AVGException.h"
#include "AVGPlayer.h"
#include "AVGLogger.h"
#include "AVGContainer.h"
#include "AVGFFMpegDecoder.h"

#include <paintlib/plbitmap.h>
#include <paintlib/pldirectfbbmp.h>
#include <paintlib/plpngenc.h>
#include <paintlib/planybmp.h>
#include <paintlib/Filter/plfilterfill.h>

#include <nsMemory.h>
#include <xpcom/nsComponentManagerUtils.h>

#include <iostream>
#include <sstream>
#include <unistd.h>

using namespace std;

NS_IMPL_ISUPPORTS2_CI(AVGVideo, IAVGNode, IAVGVideo);

bool AVGVideo::m_bInitialized = false;

AVGVideo * AVGVideo::create()
{
    return createNode<AVGVideo>("@c-base.org/avgvideo;1");
}       

AVGVideo::AVGVideo ()
    : m_State(Unloaded),
      m_pDecoder(0),
      m_pBmp(0)
{
    NS_INIT_ISUPPORTS();
    m_bFrameAvailable = false;
}

AVGVideo::~AVGVideo ()
{
    if (m_pDecoder) {
        delete m_pDecoder;
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
        *_retval = m_pDecoder->getNumFrames();
    } else {
        AVG_TRACE(IAVGPlayer::DEBUG_WARNING,
               "Error in AVGVideo::GetNumFrames: Video not loaded.");
        *_retval = -1;
    }
    return NS_OK;
}

NS_IMETHODIMP AVGVideo::GetCurFrame(int *_retval)
{
    if (m_State != Unloaded) {
        *_retval = m_CurFrame;
    } else {
        AVG_TRACE(IAVGPlayer::DEBUG_WARNING, 
                "Error in AVGVideo::GetCurFrame: Video not loaded.");
        *_retval = -1;
    }
    return NS_OK;
}

NS_IMETHODIMP AVGVideo::SeekToFrame(int num)
{
    if (m_State != Unloaded) {
        seek(num);
    } else {
        AVG_TRACE(IAVGPlayer::DEBUG_WARNING, 
                "Error in AVGVideo::SeekToFrame: Video not loaded.");
    }
    return NS_OK;
}

NS_IMETHODIMP AVGVideo::GetFPS(PRInt32 *_retval)
{
    if (m_State != Unloaded) {
        // TODO: This shouldn't be an int...
        *_retval = int(m_pDecoder->getFPS());
    } else {
        *_retval = -1;
    }
    return NS_OK;
}

void AVGVideo::init (const std::string& id, const std::string& filename,
       bool bLoop, bool bOverlay, 
       IAVGDisplayEngine * pEngine, AVGContainer * pParent, AVGPlayer * pPlayer)
{
    AVGNode::init(id, pEngine, pParent, pPlayer);
    
    m_Filename = filename;
    m_pDecoder = new AVGFFMpegDecoder();
    changeState(Paused);
//    bool bOk = m_pDecoder->open(m_Filename, &m_Width, &m_Height);

    m_bLoop = bLoop;
}

void AVGVideo::prepareRender (int time, const AVGDRect& parent)
{
    AVGNode::prepareRender(time, parent);
    if (m_State == Playing) {
        invalidate();
    }
}

void AVGVideo::render (const AVGDRect& Rect)
{
    switch(m_State) 
    {
        case Playing:
            {
                if (getEffectiveOpacity() < 0.001) {
                    return;
                }
                AVGDRect relVpt = getRelViewport();
                AVGDRect absVpt = getParent()->getAbsViewport();   
                if (getEffectiveOpacity() > 0.999 && 
                        dynamic_cast<AVGDFBDisplayEngine*>(getEngine()) &&
                        m_pDecoder->canRenderToBuffer(getEngine()->getBPP()) &&
                        relVpt.tl.x >= 0 && relVpt.tl.y >= 0 && 
                        absVpt.Width() > relVpt.br.x && absVpt.Height() > relVpt.br.y &&
                        m_Width == relVpt.Width() && m_Height == relVpt.Height())
                {
                    // Render frame to backbuffer directly.
                    // (DirectFB only, no alpha, no scale, no crop, 
                    // bpp must be supported by decoder).
                    renderToBackbuffer();
                } else {
                    renderToBmp();
                    getEngine()->blt32(m_pBmp, &getAbsViewport(), 
                            getEffectiveOpacity(), getAngle(), getPivot());
                }
            }
            break;
        case Paused:
            if (!m_bFrameAvailable) {
                renderToBmp();
            }
            getEngine()->blt32(m_pBmp, &getAbsViewport(), 
                    getEffectiveOpacity(), getAngle(), getPivot());
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
        bool m_bOk = m_pDecoder->open(m_Filename, &m_Width, &m_Height);
        m_CurFrame = 0;
        if (!m_bOk) {
            return;
        }
        m_pBmp = getEngine()->createSurface();
        AVGDRect vpt = getRelViewport();
        
        m_pBmp->Create(m_Width, m_Height, 24, false, false);
        m_bFrameAvailable = false;
    }
    if (NewState == Unloaded) {
        m_pDecoder->close();
        delete m_pBmp;
        m_pBmp = 0;
    }
    addDirtyRect(getVisibleRect());
    m_State = NewState;
}

void AVGVideo::seek(int DestFrame) {
    m_pDecoder->seek(DestFrame, m_CurFrame);
    m_CurFrame = DestFrame;
    m_bFrameAvailable = false;
}

void dumpBmpLineArray(PLBmp* pBmp)
{
    PLBYTE ** ppLines = pBmp->GetLineArray();
    for (int y = 0; y < pBmp->GetHeight(); y++) {
        cerr << "Line " << y << ": " << hex << (void*)ppLines[y] << dec << endl;
    }
}

void AVGVideo::renderToBmp()
{
    m_bEOF = m_pDecoder->renderToBmp(m_pBmp);

    if (!m_bEOF) {
        m_bFrameAvailable = true;
        getEngine()->surfaceChanged(m_pBmp);
    }
    advancePlayback();
}

void AVGVideo::renderToBackbuffer()
{
    AVGDFBDisplayEngine* pEngine = 
        dynamic_cast<AVGDFBDisplayEngine*>(getEngine());
    AVGDRect vpt = getVisibleRect();
    // Calc row ptr array.
    IDirectFBSurface * pSurface = pEngine->getPrimary();
    PLBYTE * pSurfBits;
    int Pitch;
    DFBResult err = pSurface->Lock(pSurface, 
            DFBSurfaceLockFlags(DSLF_WRITE), (void **)&pSurfBits, &Pitch);
    pEngine->DFBErrorCheck(AVG_ERR_DFB, 
            "AVGFFMpegDecoder::renderToBackbuffer", err);
    // TODO: 16 bpp support.
    m_bEOF = m_pDecoder->renderToBuffer(pSurfBits, Pitch, 
            getEngine()->getBPP()/8, vpt);
    pSurface->Unlock(pSurface);

    m_bFrameAvailable=false;
    advancePlayback();

}

bool AVGVideo::obscures (const AVGDRect& Rect, int z)
{
    return (getEffectiveOpacity() > 0.999 &&
            getZ() > z && getVisibleRect().Contains(Rect));

}

void AVGVideo::advancePlayback()
{
    m_CurFrame++;
    if (m_bEOF) {
        if (m_bLoop) {
            seek(0);
        } else {
            changeState(Paused);
        }
    }
}

string AVGVideo::dump (int indent)
{
/*
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
*/
    return "";
}

AVGDPoint AVGVideo::getPreferredMediaSize()
{
    return AVGDPoint(m_Width, m_Height);
}


