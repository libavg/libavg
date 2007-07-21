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
      m_bFrameAvailable(false),
      m_bFirstFrameDecoded(false)
{
}

VideoBase::VideoBase (const xmlNodePtr xmlNode, Player * pPlayer)
    : RasterNode(xmlNode, pPlayer),
      m_VideoState(Unloaded),
      m_bFrameAvailable(false),
      m_bFirstFrameDecoded(false)
{
}

VideoBase::~VideoBase ()
{
}

void VideoBase::setDisplayEngine(DisplayEngine * pEngine)
{
    RasterNode::setDisplayEngine(pEngine);
    VideoState TempVideoState = m_VideoState;
    m_VideoState = Unloaded;
    try {
        changeVideoState(TempVideoState);
    } catch (Exception& ex) {
        AVG_TRACE(Logger::WARNING, ex.GetStr());
    }
}

void VideoBase::disconnect()
{
    changeVideoState(Unloaded);
    Node::disconnect();
}

void VideoBase::play()
{
    changeVideoState(Playing);
}

void VideoBase::stop()
{
    changeVideoState(Unloaded);
}

void VideoBase::pause()
{
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
                bool bNewFrame = renderToSurface(getSurface());
                m_bFrameAvailable = m_bFrameAvailable | bNewFrame;
                if (m_bFrameAvailable) {
                    m_bFirstFrameDecoded = true;
                }
                if (m_bFirstFrameDecoded) {
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
            if (m_bFrameAvailable) {
                m_bFirstFrameDecoded = true;
            }
            if (m_bFirstFrameDecoded) {
                getEngine()->blt32(getSurface(), &getAbsViewport(), 
                        getEffectiveOpacity(), getAngle(), getPivot(),
                        getBlendMode());
            }
            break;
        case Unloaded:
            break;
    }
}

void VideoBase::changeVideoState(VideoState NewVideoState)
{
    if (isDisplayAvailable()) {
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
    }
    m_VideoState = NewVideoState;
}

void VideoBase::open() 
{
    open(getEngine()->getYCbCrMode());
    setViewport(-32767, -32767, -32767, -32767);
    PixelFormat pf = getPixelFormat();
    getSurface()->create(getSize(), pf, true);
    if (pf == B8G8R8X8 || pf == B8G8R8A8) {
        FilterFill<Pixel32> Filter(Pixel32(0,0,0,255));
        Filter.applyInPlace(getSurface()->lockBmp());
        getSurface()->unlockBmps();
    }

    m_bFirstFrameDecoded = false;
    m_bFrameAvailable = false;
}

int VideoBase::getMediaWidth()
{
    return getSize().x;
}

int VideoBase::getMediaHeight()
{
    return getSize().y;
}

bool VideoBase::obscures (const DRect& Rect, int Child)
{
    return (m_bFirstFrameDecoded && isActive() && getEffectiveOpacity() > 0.999 &&
            getVisibleRect().Contains(Rect) && m_VideoState != Unloaded);
}

string VideoBase::dump (int indent)
{
    return "";
}

DPoint VideoBase::getPreferredMediaSize()
{
    return DPoint(getSize());
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
