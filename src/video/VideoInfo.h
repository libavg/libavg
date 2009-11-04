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

#ifndef _VideoInfo_H_
#define _VideoInfo_H_

#include "../api.h"

#include "../base/Point.h"

#include <string>

namespace avg {

struct AVG_API VideoInfo
{
    VideoInfo();
    VideoInfo(long long duration, int bitrate, bool bHasVideo, bool bHasAudio);
    void setVideoData(const IntPoint& size, const std::string& sPixelFormat,
            int numFrames, double streamFPS, double FPS, const std::string& sVCodec);
    void setAudioData(const std::string& sACodec, int sampleRate, int numAudioChannels);

    long long m_Duration;
    int m_Bitrate;

    bool m_bHasVideo;
    IntPoint m_Size;
    std::string m_sPixelFormat;
    int m_NumFrames;
    double m_StreamFPS;
    double m_FPS;
    std::string m_sVCodec;

    bool m_bHasAudio;
    std::string m_sACodec;
    int m_SampleRate;
    int m_NumAudioChannels;
};

}
#endif 

