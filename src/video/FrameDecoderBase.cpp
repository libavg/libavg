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

#include "FrameDecoderBase.h"

namespace avg {

FrameDecoderBase::FrameDecoderBase():
    m_bEOF(false),
    m_StartTimestamp(-1),
    m_LastFrameTime(-1),
    m_bUseStreamFPS(true)
{

}

FrameDecoderBase::~FrameDecoderBase()
{

}

float FrameDecoderBase::getCurTime() const
{
    return m_LastFrameTime;
}

float FrameDecoderBase::getFPS() const
{
    return m_FPS;
}

void FrameDecoderBase::setFPS(float fps)
{
    m_FPS = fps;
}

bool FrameDecoderBase::isEOF() const
{
    return m_bEOF;
}

float FrameDecoderBase::getFrameTime(long long dts, bool bFrameAfterSeek)
{
    //TODO: getFrameTime
    return 0;
}

}
