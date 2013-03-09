//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2011 Ulrich von Zadow
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

#include "VideoMsg.h"
#include "WrapFFMpeg.h"
#ifdef AVG_ENABLE_VAAPI
#include "VAAPISurface.h"
#endif

#include "../base/ObjectCounter.h"
#include "../base/Exception.h"

namespace avg {

VideoMsg::VideoMsg()
{
}

VideoMsg::~VideoMsg()
{
    if (getType() == VAAPI_FRAME) {
        m_pSurface->decRef();
    }
}

void VideoMsg::setFrame(const std::vector<BitmapPtr>& pBmps, float frameTime)
{
    AVG_ASSERT(pBmps.size() == 1 || pBmps.size() == 3 || pBmps.size() == 4);
    setType(FRAME);
    m_pBmps = pBmps;
    m_FrameTime = frameTime;
}

void VideoMsg::setVDPAUFrame(vdpau_render_state* pRenderState, float frameTime)
{
    setType(VDPAU_FRAME);
    m_pRenderState = pRenderState;
    m_FrameTime = frameTime;
}

void VideoMsg::setVAAPIFrame(VAAPISurface* pSurface, float frameTime)
{
    setType(VAAPI_FRAME);
    pSurface->incRef();
    m_pSurface = pSurface;
    m_FrameTime = frameTime;
}
    
void VideoMsg::setPacket(AVPacket* pPacket)
{
    setType(PACKET);
    AVG_ASSERT(pPacket);
    m_pPacket = pPacket;
}

AVPacket * VideoMsg::getPacket() const
{
    AVG_ASSERT(getType() == PACKET);
    return m_pPacket;
}

void VideoMsg::freePacket()
{
    if (getType() == PACKET) {
        av_free_packet(m_pPacket);
        delete m_pPacket;
        m_pPacket = 0;
    }
}

BitmapPtr VideoMsg::getFrameBitmap(int i) const
{
    AVG_ASSERT(getType() == FRAME);
    return m_pBmps[i];
}

float VideoMsg::getFrameTime() const
{
    AVG_ASSERT(getType() == FRAME || getType() == VDPAU_FRAME || getType() == VAAPI_FRAME);
    return m_FrameTime;
}

vdpau_render_state* VideoMsg::getRenderState() const
{
    AVG_ASSERT(getType() == VDPAU_FRAME);
    return m_pRenderState;
}
    
VAAPISurface* VideoMsg::getVAAPISurface() const
{
    AVG_ASSERT(getType() == VAAPI_FRAME);
    return m_pSurface;
}

}

