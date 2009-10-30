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

#include "VideoInfo.h"

namespace avg {

using namespace std;

VideoInfo::VideoInfo()
{
}

VideoInfo::VideoInfo(long long duration, int bitrate, bool bHasVideo, bool bHasAudio)
    : m_Duration(duration),
      m_Bitrate(bitrate),
      m_bHasVideo(bHasVideo),
      m_bHasAudio(bHasAudio)
{
}

void VideoInfo::setVideoData(const IntPoint& size, int numFrames, double streamFPS, 
        double FPS, char vCodec[4])
{
    m_Size = size;
    m_NumFrames = numFrames;
    m_StreamFPS = streamFPS;
    m_FPS = FPS;
    m_sVCodec = string(vCodec, 4);
}

}
