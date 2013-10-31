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

#ifndef _VideoInfo_H_
#define _VideoInfo_H_

#include "../api.h"

#include "WrapFFMpeg.h"

#include "../base/GLMHelper.h"

#include <string>

namespace avg {

struct AVG_API VideoInfo
{
    VideoInfo();
    VideoInfo(std::string sContainerFormat, float duration, int bitrate, bool bHasVideo,
            bool bHasAudio);
    void setVideoData(const IntPoint& size, const std::string& sPixelFormat,
            int numFrames, float streamFPS, const std::string& sVCodec,
            bool bUsesVDPAU, float duration);

    void setAudioData(const std::string& sACodec, int sampleRate, int numAudioChannels,
            float duration);

    std::string m_sContainerFormat;
    float m_Duration;
    int m_Bitrate;

    bool m_bHasVideo;
    IntPoint m_Size;
    std::string m_sPixelFormat;
    int m_NumFrames;
    float m_StreamFPS;
    std::string m_sVCodec;
    bool m_bUsesVDPAU;
    float m_VideoDuration;

    bool m_bHasAudio;
    std::string m_sACodec;
    int m_SampleRate;
    int m_NumAudioChannels;
    float m_AudioDuration;
};

float getStreamFPS(AVStream* pStream);

}
#endif 

