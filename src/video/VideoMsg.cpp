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

#include "../base/ObjectCounter.h"
#include "../base/Exception.h"

namespace avg {

VideoMsg::VideoMsg()
    : m_MsgType(NONE)
{
    ObjectCounter::get()->incRef(&typeid(*this));
}

VideoMsg::~VideoMsg()
{
    ObjectCounter::get()->decRef(&typeid(*this));
}

void VideoMsg::setAudio(AudioBufferPtr pAudioBuffer, double audioTime)
{
    AVG_ASSERT(m_MsgType == NONE);
    AVG_ASSERT(pAudioBuffer);
    m_MsgType = AUDIO;
    m_pAudioBuffer = pAudioBuffer;
    m_AudioTime = audioTime;
}

void VideoMsg::setEOF()
{
    AVG_ASSERT(m_MsgType == NONE);
    m_MsgType = END_OF_FILE;
}

void VideoMsg::setError(const Exception& ex)
{
    AVG_ASSERT(m_MsgType == NONE);
    m_MsgType = ERROR;
    m_pEx = new Exception(ex);
}

void VideoMsg::setFrame(const std::vector<BitmapPtr>& pBmps, double frameTime)
{
    AVG_ASSERT(m_MsgType == NONE);
    AVG_ASSERT(pBmps.size() == 1 || pBmps.size() == 3 || pBmps.size() == 4);
    m_MsgType = FRAME;
    m_pBmps = pBmps;
    m_FrameTime = frameTime;
}

void VideoMsg::setVDPAUFrame(vdpau_render_state* pRenderState, double frameTime)
{
    AVG_ASSERT(m_MsgType == NONE);
    m_MsgType = VDPAU_FRAME;
    m_pRenderState = pRenderState;
    m_FrameTime = frameTime;
}

void VideoMsg::setSeekDone(double seekVideoFrameTime, double seekAudioFrameTime)
{
    AVG_ASSERT(m_MsgType == NONE);
    m_MsgType = SEEK_DONE;
    m_SeekVideoFrameTime = seekVideoFrameTime;
    m_SeekAudioFrameTime = seekAudioFrameTime;
}

VideoMsg::MsgType VideoMsg::getType()
{
    return m_MsgType;
}

AudioBufferPtr VideoMsg::getAudioBuffer() const
{
    AVG_ASSERT(m_MsgType == AUDIO);
    return m_pAudioBuffer;
}

double VideoMsg::getAudioTime() const
{
    AVG_ASSERT(m_MsgType == AUDIO);
    return m_AudioTime;
}

const Exception& VideoMsg::getException() const
{
    AVG_ASSERT(m_MsgType == ERROR);
    return *m_pEx;
}

BitmapPtr VideoMsg::getFrameBitmap(int i)
{
    AVG_ASSERT(m_MsgType == FRAME);
    return m_pBmps[i];
}

double VideoMsg::getFrameTime()
{
    AVG_ASSERT(m_MsgType == FRAME || m_MsgType == VDPAU_FRAME);
    return m_FrameTime;
}

double VideoMsg::getSeekVideoFrameTime()
{
    AVG_ASSERT(m_MsgType == SEEK_DONE);
    return m_SeekVideoFrameTime;
}

double VideoMsg::getSeekAudioFrameTime()
{
    AVG_ASSERT(m_MsgType == SEEK_DONE);
    return m_SeekAudioFrameTime;
}

vdpau_render_state* VideoMsg::getRenderState()
{
    AVG_ASSERT(m_MsgType == VDPAU_FRAME);
    return m_pRenderState;
}

}

