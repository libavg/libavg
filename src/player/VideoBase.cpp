//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2006 Ulrich von Zadow
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

#include "../avgconfig.h"
#undef PACKAGE_BUGREPORT
#undef PACKAGE_NAME
#undef PACKAGE_STRING
#undef PACKAGE_TARNAME
#undef PACKAGE_VERSION

#include "VideoBase.h"
#include "DivNode.h"
#include "DisplayEngine.h"
#ifdef AVG_ENABLE_DFB
#include "DFBDisplayEngine.h"
#include "DFBSurface.h"
#endif
#include "Player.h"
#include "ISurface.h"

#include "../base/Exception.h"
#include "../base/Logger.h"

#include "../graphics/Filterfill.h"
#include "../graphics/Pixel24.h"

#include <iostream>
#include <sstream>
#include <unistd.h>

using namespace std;

namespace avg {

VideoBase::VideoBase ()
    : m_VideoState(Unloaded),
      m_Width(0),
      m_Height(0),
      m_YCbCrMode(DisplayEngine::NONE),
      m_bFrameAvailable(false)
{
}

VideoBase::VideoBase (const xmlNodePtr xmlNode, Player * pPlayer)
    : RasterNode(xmlNode, pPlayer),
      m_VideoState(Unloaded),
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
    if (getState() != NS_CONNECTED) {
        throw Exception(AVG_ERR_NOT_IN_SCENE,
                "VideoBase::play() called on object not in scene.");
    }
    changeVideoState(Playing);
}

void VideoBase::stop()
{
    if (getState() != NS_CONNECTED) {
        throw Exception(AVG_ERR_NOT_IN_SCENE,
                "VideoBase::stop() called on object not in scene.");
    }
    changeVideoState(Unloaded);
}

void VideoBase::pause()
{
    if (getState() != NS_CONNECTED) {
        throw Exception(AVG_ERR_NOT_IN_SCENE,
                "VideoBase::pause() called on object not in scene.");
    }
    changeVideoState(Paused);
}

void VideoBase::prepareRender (int time, const DRect& parent)
{
    Node::prepareRender(time, parent);
    if (m_VideoState == Playing) {
        invalidate();
    }
}

void VideoBase::render (const DRect& Rect)
{
    switch(m_VideoState) 
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

void VideoBase::changeVideoState(VideoState NewVideoState)
{
    if (m_VideoState == NewVideoState) {
        return;
    }
    if (m_VideoState == Unloaded) {
        open();
    }
    if (NewVideoState == Unloaded) {
        close();
    }
    addDirtyRect(getVisibleRect());
    m_VideoState = NewVideoState;
}

void VideoBase::renderToBackbuffer()
{
#ifdef AVG_ENABLE_DFB
    DFBDisplayEngine* pEngine = 
        dynamic_cast<DFBDisplayEngine*>(getEngine());
    DRect vpt = getVisibleRect();
    IDirectFBSurface * pSurface = pEngine->getPrimary();
    unsigned char * pSurfBits;
    int Pitch;
    DFBResult err = pSurface->Lock(pSurface, 
            DFBSurfaceLockFlags(DSLF_WRITE), (void **)&pSurfBits, &Pitch);
    pEngine->DFBErrorCheck(AVG_ERR_DFB, 
            "VideoBase::renderToBackbuffer", err);
    DFBSurface SubSurface;
    IntRect plvpt = IntRect(vpt);
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
    setViewport(-32767, -32767, -32767, -32767);

    DRect vpt = getRelViewport();
    m_YCbCrMode = DisplayEngine::NONE;
    if (getDesiredPixelFormat() == YCbCr420p && 
            getEngine()->getYCbCrMode() != DisplayEngine::NONE) 
    {
        m_YCbCrMode = getEngine()->getYCbCrMode();
    }
    switch (m_YCbCrMode) {
        case DisplayEngine::OGL_MESA:
        case DisplayEngine::OGL_APPLE:
            getSurface()->create(IntPoint(m_Width, m_Height), YCbCr422, true);
            break;
        case DisplayEngine::OGL_SHADER:
            getSurface()->create(IntPoint(m_Width, m_Height), YCbCr420p, true);
            break;
        case DisplayEngine::NONE:
            {
                PixelFormat pf;
                if (getDesiredPixelFormat() == R8G8B8X8) {
                    if (getEngine()->hasRGBOrdering()) {
                        pf = R8G8B8X8;
                    } else {
                        pf = B8G8R8X8;
                    }
                    getSurface()->create(IntPoint(m_Width, m_Height), pf, true);
                    FilterFill<Pixel24> Filter(Pixel24(0,0,0));
                    Filter.applyInPlace(getSurface()->lockBmp());
                } else { 
                    if (getEngine()->hasRGBOrdering()) {
                        pf = R8G8B8A8;
                    } else {
                        pf = B8G8R8A8;
                    }
                    getSurface()->create(IntPoint(m_Width, m_Height), pf, true);
                    FilterFill<Pixel32> Filter(Pixel32(0,0,0, 255));
                    Filter.applyInPlace(getSurface()->lockBmp());
                }
                getSurface()->unlockBmps();
            }
            break;
    }
    
    m_bFrameAvailable = false;
    m_VideoState = Paused;
}

int VideoBase::getMediaWidth()
{
    return m_Width;
}

int VideoBase::getMediaHeight()
{
    return m_Height;
}

DisplayEngine::YCbCrMode VideoBase::getYCbCrMode()
{
    return m_YCbCrMode;
}

bool VideoBase::obscures (const DRect& Rect, int Child)
{
    return (isActive() && getEffectiveOpacity() > 0.999 &&
            getVisibleRect().Contains(Rect) && m_VideoState != Unloaded);
}

string VideoBase::dump (int indent)
{
    return "";
}

DPoint VideoBase::getPreferredMediaSize()
{
    return DPoint(m_Width, m_Height);
}

VideoBase::VideoState VideoBase::getVideoState() const
{
    return m_VideoState;
}

void VideoBase::setFrameAvailable(bool bAvailable)
{
    m_bFrameAvailable = bAvailable;
}

}
