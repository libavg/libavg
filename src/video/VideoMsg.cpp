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

void VideoMsg::setAudio(AudioBufferPtr pAudioBuffer, long long audioTime)
{
    m_MsgType = AUDIO;
    m_pAudioBuffer = pAudioBuffer;
    m_AudioTime = audioTime;
}

void VideoMsg::setEOF()
{
    m_MsgType = END_OF_FILE;
}

void VideoMsg::setError(const Exception& ex)
{
    m_MsgType = ERROR;
    m_pEx = new Exception(ex);
}

void VideoMsg::setFrame(const std::vector<BitmapPtr>& pBmps, long long frameTime)
{
    m_MsgType = FRAME;
    m_pBmps = pBmps;
    m_FrameTime = frameTime;
}

void VideoMsg::setSeekDone(long long seekVideoFrameTime, long long seekAudioFrameTime)
{
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

long long VideoMsg::getAudioTime() const
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

long long VideoMsg::getFrameTime()
{
    AVG_ASSERT(m_MsgType == FRAME);
    return m_FrameTime;
}

long long VideoMsg::getSeekVideoFrameTime()
{
    AVG_ASSERT(m_MsgType == SEEK_DONE);
    return m_SeekVideoFrameTime;
}

long long VideoMsg::getSeekAudioFrameTime()
{
    AVG_ASSERT(m_MsgType == SEEK_DONE);
    return m_SeekAudioFrameTime;
}

}

