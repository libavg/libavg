//
// $Id$
// 

#include "avgconfig.h"
#include "AVGVideoBase.h"
#include "IAVGDisplayEngine.h"
#ifdef AVG_ENABLE_DFB
#include "AVGDFBDisplayEngine.h"
#endif
#include "AVGException.h"
#include "AVGPlayer.h"
#include "AVGLogger.h"
#include "AVGContainer.h"

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

NS_IMPL_ISUPPORTS2(AVGVideoBase, IAVGNode, IAVGVideoBase);

AVGVideoBase::AVGVideoBase ()
    : m_State(Unloaded),
      m_pBmp(0)
{
    NS_INIT_ISUPPORTS();
    m_bFrameAvailable = false;
}

AVGVideoBase::~AVGVideoBase ()
{
    if (m_pBmp) {
        delete m_pBmp;
    }
}

NS_IMETHODIMP 
AVGVideoBase::Play()
{
    changeState(Playing);
    return NS_OK;
}

NS_IMETHODIMP AVGVideoBase::Stop()
{
    changeState(Unloaded);
    return NS_OK;
}

NS_IMETHODIMP AVGVideoBase::Pause()
{
    changeState(Paused);
    return NS_OK;
}

NS_IMETHODIMP AVGVideoBase::GetFPS(PRInt32 *_retval)
{
    if (m_State != Unloaded) {
        // TODO: This shouldn't be an int...
        *_retval = int(getFPS());
    } else {
        *_retval = -1;
    }
    return NS_OK;
}

void AVGVideoBase::init (const std::string& id, bool bOverlay, 
       IAVGDisplayEngine * pEngine, AVGContainer * pParent, AVGPlayer * pPlayer)
{
    AVGNode::init(id, pEngine, pParent, pPlayer);
    
    changeState(Paused);
}

void AVGVideoBase::prepareRender (int time, const AVGDRect& parent)
{
    AVGNode::prepareRender(time, parent);
    if (m_State == Playing) {
        invalidate();
    }
}

void AVGVideoBase::render (const AVGDRect& Rect)
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
#ifdef AVG_ENABLE_DFB
                if (getEffectiveOpacity() > 0.999 && 
                        dynamic_cast<AVGDFBDisplayEngine*>(getEngine()) &&
                        canRenderToBackbuffer(getEngine()->getBPP()) &&
                        relVpt.tl.x >= 0 && relVpt.tl.y >= 0 && 
                        absVpt.Width() >= relVpt.br.x && absVpt.Height() >= relVpt.br.y &&
                        m_Width == relVpt.Width() && m_Height == relVpt.Height())
                {
                    // Render frame to backbuffer directly.
                    // (DirectFB only, no alpha, no scale, no crop, 
                    // bpp must be supported by decoder).
                    renderToBackbuffer();
                } else
#endif                
                {
                    
                    m_bFrameAvailable = renderToBmp(m_pBmp);
                    getEngine()->blt32(m_pBmp, &getAbsViewport(), 
                            getEffectiveOpacity(), getAngle(), getPivot());
                }
            }
            break;
        case Paused:
            if (!m_bFrameAvailable) {
                m_bFrameAvailable = renderToBmp(m_pBmp);
            }
            getEngine()->blt32(m_pBmp, &getAbsViewport(), 
                    getEffectiveOpacity(), getAngle(), getPivot());
            break;
        case Unloaded:
            break;
    }
}

void AVGVideoBase::changeState(VideoState NewState)
{
    if (m_State == NewState) {
        return;
    }
    if (m_State == Unloaded) {
        bool m_bOk = open(&m_Width, &m_Height);
        if (!m_bOk) {
            return;
        }
        m_pBmp = getEngine()->createSurface();
        AVGDRect vpt = getRelViewport();
        
        m_pBmp->Create(m_Width, m_Height, 24, false, false);
        m_bFrameAvailable = false;
    }
    if (NewState == Unloaded) {
        close();
        delete m_pBmp;
        m_pBmp = 0;
    }
    addDirtyRect(getVisibleRect());
    m_State = NewState;
}

void AVGVideoBase::renderToBackbuffer()
{
#ifdef AVG_ENABLE_DFB
    AVGDFBDisplayEngine* pEngine = 
        dynamic_cast<AVGDFBDisplayEngine*>(getEngine());
    AVGDRect vpt = getVisibleRect();
    IDirectFBSurface * pSurface = pEngine->getPrimary();
    PLBYTE * pSurfBits;
    int Pitch;
    DFBResult err = pSurface->Lock(pSurface, 
            DFBSurfaceLockFlags(DSLF_WRITE), (void **)&pSurfBits, &Pitch);
    pEngine->DFBErrorCheck(AVG_ERR_DFB, 
            "AVGVideoBase::renderToBackbuffer", err);
    // TODO: 16 bpp support.
    renderToBackbuffer(pSurfBits, Pitch, 
            getEngine()->getBPP()/8, vpt);
    pSurface->Unlock(pSurface);

    m_bFrameAvailable=false;
#else
    AVG_TRACE(AVGPlayer::DEBUG_ERROR, 
            "renderToBackbuffer called unexpectedly. Aborting.");
    exit(-1);
#endif    
}

int AVGVideoBase::getMediaWidth()
{
    return m_Width;
}

int AVGVideoBase::getMediaHeight()
{
    return m_Height;
}

bool AVGVideoBase::obscures (const AVGDRect& Rect, int z)
{
    return (getEffectiveOpacity() > 0.999 &&
            getZ() > z && getVisibleRect().Contains(Rect));

}

string AVGVideoBase::dump (int indent)
{
    return "";
}

AVGDPoint AVGVideoBase::getPreferredMediaSize()
{
    return AVGDPoint(m_Width, m_Height);
}

AVGVideoBase::VideoState AVGVideoBase::getState()
{
    return m_State;
}

void AVGVideoBase::setFrameAvailable(bool bAvailable)
{
    m_bFrameAvailable = bAvailable;
}

