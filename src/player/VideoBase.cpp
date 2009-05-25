//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2008 Ulrich von Zadow
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

#include "VideoBase.h"
#include "../avgconfigwrapper.h"

#include "DivNode.h"
#include "SDLDisplayEngine.h"
#include "Player.h"
#include "NodeDefinition.h"

#include "../base/Exception.h"
#include "../base/Logger.h"

#include "../graphics/Filterfill.h"
#include "../graphics/Pixel24.h"

#include <iostream>
#include <sstream>

#ifndef _WIN32
#include <unistd.h>
#endif

using namespace std;

namespace avg {

NodeDefinition VideoBase::createDefinition()
{
    return NodeDefinition("videobase")
        .extendDefinition(RasterNode::createDefinition());
}

VideoBase::VideoBase()
    : m_VideoState(Unloaded),
      m_bFrameAvailable(false),
      m_bFirstFrameDecoded(false)
{
}

VideoBase::~VideoBase ()
{
}

void VideoBase::setRenderingEngines(DisplayEngine * pDisplayEngine, AudioEngine * pAudioEngine)
{
    RasterNode::setRenderingEngines(pDisplayEngine, pAudioEngine);
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
    RasterNode::disconnect();
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

void VideoBase::render(const DRect& Rect)
{
    switch(m_VideoState) 
    {
        case Playing:
            {
                bool bNewFrame = renderToSurface(getSurface());
                m_bFrameAvailable = m_bFrameAvailable | bNewFrame;
                if (m_bFrameAvailable) {
                    m_bFirstFrameDecoded = true;
                }
                if (m_bFirstFrameDecoded) {
                    getSurface()->blt32(getSize(), getEffectiveOpacity(), getBlendMode());
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
                getSurface()->blt32(getSize(), getEffectiveOpacity(), getBlendMode());
            }
            break;
        case Unloaded:
            break;
    }
}

void VideoBase::changeVideoState(VideoState NewVideoState)
{
    if (getState() == NS_CANRENDER) {
        if (m_VideoState == NewVideoState) {
            return;
        }
        if (m_VideoState == Unloaded) {
            open();
        }
        if (NewVideoState == Unloaded) {
            close();
        }
    }
    m_VideoState = NewVideoState;
}

void VideoBase::open() 
{
    open(getDisplayEngine()->isUsingShaders());
    setViewport(-32767, -32767, -32767, -32767);
    PixelFormat pf = getPixelFormat();
    getSurface()->create(getDisplayEngine(), getMediaSize(), pf, true);
    if (pf == B8G8R8X8 || pf == B8G8R8A8) {
        FilterFill<Pixel32> Filter(Pixel32(0,0,0,255));
        Filter.applyInPlace(getSurface()->lockBmp());
        getSurface()->unlockBmps();
    }

    m_bFirstFrameDecoded = false;
    m_bFrameAvailable = false;
}

string VideoBase::dump (int indent)
{
    return "";
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
