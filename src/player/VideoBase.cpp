//
// $Id$
// 

#include "avgconfig.h"
#include "VideoBase.h"
#include "IDisplayEngine.h"
#ifdef AVG_ENABLE_DFB
#include "DFBDisplayEngine.h"
#include "DFBSurface.h"
#endif
#include "Player.h"
#include "Container.h"
#include "ISurface.h"

#include "../base/Exception.h"
#include "../base/Logger.h"

#include <paintlib/plbitmap.h>
#include <paintlib/plpngenc.h>
#include <paintlib/planybmp.h>
#include <paintlib/Filter/plfilterfill.h>

#include <iostream>
#include <sstream>
#include <unistd.h>

using namespace std;

namespace avg {

VideoBase::VideoBase ()
    : m_State(Unloaded),
      m_Width(0),
      m_Height(0),
      m_bFrameAvailable(false)
{
}

VideoBase::~VideoBase ()
{
}

void VideoBase::play()
{
    if (!isInitialized()) {
        throw Exception(AVG_ERR_NOT_IN_SCENE,
                "VideoBase::play() called on object not in scene.");
    }
    changeState(Playing);
}

void VideoBase::stop()
{
    if (!isInitialized()) {
        throw Exception(AVG_ERR_NOT_IN_SCENE,
                "VideoBase::stop() called on object not in scene.");
    }
    changeState(Unloaded);
}

void VideoBase::pause()
{
    if (!isInitialized()) {
        throw Exception(AVG_ERR_NOT_IN_SCENE,
                "VideoBase::pause() called on object not in scene.");
    }
    changeState(Paused);
}

void VideoBase::init(IDisplayEngine * pEngine, Container * pParent, 
        Player * pPlayer)
{
    Node::init(pEngine, pParent, pPlayer);
    open();
}

void VideoBase::prepareRender (int time, const DRect& parent)
{
    Node::prepareRender(time, parent);
    if (m_State == Playing) {
        invalidate();
    }
}

void VideoBase::render (const DRect& Rect)
{
    switch(m_State) 
    {
        case Playing:
            {
                if (getEffectiveOpacity() < 0.001) {
                    return;
                }
                DRect relVpt = getRelViewport();
                DRect absVpt = getParent()->getAbsViewport();   
#ifdef AVG_ENABLE_DFB
                if (getEffectiveOpacity() > 0.999 && 
                        dynamic_cast<DFBDisplayEngine*>(getEngine()) &&
                        canRenderToBackbuffer(getEngine()->getBPP()) &&
                        relVpt.tl.x >= 0 && relVpt.tl.y >= 0 && 
                        absVpt.Width() >= relVpt.br.x && 
                        absVpt.Height() >= relVpt.br.y &&
                        m_Width == relVpt.Width() && m_Height == relVpt.Height())
                {
                    // Render frame to backbuffer directly.
                    // (DirectFB only, no alpha, no scale, no crop, 
                    // bpp must be supported by decoder).
                    renderToBackbuffer();
                } else
#endif                
                {
                    m_bFrameAvailable = renderToSurface(getSurface());
                    getEngine()->blt32(getSurface(), &getAbsViewport(), 
                            getEffectiveOpacity(), getAngle(), getPivot(),
                            getBlendMode());
                }
            }
            break;
        case Paused:
            if (!m_bFrameAvailable) {
                m_bFrameAvailable = renderToSurface(getSurface());
            }
            getEngine()->blt32(getSurface(), &getAbsViewport(), 
                    getEffectiveOpacity(), getAngle(), getPivot(),
                    getBlendMode());
            break;
        case Unloaded:
            break;
    }
}

void VideoBase::changeState(VideoState NewState)
{
    if (m_State == NewState) {
        return;
    }
    if (m_State == Unloaded) {
        open();
    }
    if (NewState == Unloaded) {
        close();
    }
    addDirtyRect(getVisibleRect());
    m_State = NewState;
}

void VideoBase::renderToBackbuffer()
{
#ifdef AVG_ENABLE_DFB
    DFBDisplayEngine* pEngine = 
        dynamic_cast<DFBDisplayEngine*>(getEngine());
    DRect vpt = getVisibleRect();
    IDirectFBSurface * pSurface = pEngine->getPrimary();
    PLBYTE * pSurfBits;
    int Pitch;
    DFBResult err = pSurface->Lock(pSurface, 
            DFBSurfaceLockFlags(DSLF_WRITE), (void **)&pSurfBits, &Pitch);
    pEngine->DFBErrorCheck(AVG_ERR_DFB, 
            "VideoBase::renderToBackbuffer", err);
    DFBSurface SubSurface;
    PLRect plvpt = PLRect(vpt);
    SubSurface.createFromDFBSurface(pSurface, &plvpt);
    renderToSurface(&SubSurface);
    pSurface->Unlock(pSurface);

    m_bFrameAvailable=false;
#else
    AVG_TRACE(Logger::ERROR, 
            "renderToBackbuffer called unexpectedly. Aborting.");
    exit(-1);
#endif    
}

void VideoBase::open() 
{
    open(&m_Width, &m_Height);

    DRect vpt = getRelViewport();
    getSurface()->create(m_Width, m_Height, PLPixelFormat::R8G8B8);

    PLFilterFill<PLPixel24> Filter(PLPixel24(0,0,0));
    Filter.ApplyInPlace(getSurface()->getBmp());
    
    m_bFrameAvailable = false;
    m_State = Paused;
}

int VideoBase::getMediaWidth()
{
    return m_Width;
}

int VideoBase::getMediaHeight()
{
    return m_Height;
}

bool VideoBase::obscures (const DRect& Rect, int z)
{
    return (isActive() && getEffectiveOpacity() > 0.999 &&
            getZ() > z && getVisibleRect().Contains(Rect));

}

string VideoBase::dump (int indent)
{
    return "";
}

DPoint VideoBase::getPreferredMediaSize()
{
    return DPoint(m_Width, m_Height);
}

VideoBase::VideoState VideoBase::getState()
{
    return m_State;
}

void VideoBase::setFrameAvailable(bool bAvailable)
{
    m_bFrameAvailable = bAvailable;
}

}
