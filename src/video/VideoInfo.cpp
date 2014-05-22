//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2014 Ulrich von Zadow
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

#include "../base/Exception.h"

namespace avg {

using namespace std;

VideoInfo::VideoInfo()
{
}

VideoInfo::VideoInfo(string sContainerFormat, float duration, int bitrate, bool bHasVideo,
        bool bHasAudio)
    : m_sContainerFormat(sContainerFormat), 
      m_Duration(duration),
      m_Bitrate(bitrate),
      m_bHasVideo(bHasVideo),
      m_bHasAudio(bHasAudio)
{
}

void VideoInfo::setVideoData(const IntPoint& size, const string& sPixelFormat,
        int numFrames, float streamFPS, const string& sVCodec,
        bool bUsesVDPAU, float duration)
{
    AVG_ASSERT(m_bHasVideo);
    m_Size = size;
    m_sPixelFormat = sPixelFormat;
    m_NumFrames = numFrames;
    m_StreamFPS = streamFPS;
    m_sVCodec = sVCodec;
    m_bUsesVDPAU = bUsesVDPAU;
    m_VideoDuration = duration;
}

void VideoInfo::setAudioData(const string& sACodec, int sampleRate, int numAudioChannels,
        float duration)
{
    AVG_ASSERT(m_bHasAudio);
    m_sACodec = sACodec;
    m_SampleRate = sampleRate;
    m_NumAudioChannels = numAudioChannels;
    m_AudioDuration = duration;
}
    
float getStreamFPS(AVStream* pStream)
{
    float fps = 0;
    if (pStream->avg_frame_rate.den != 0) {
        fps = float(av_q2d(pStream->avg_frame_rate));
    }
#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(54, 25, 0)
    if (fps == 0 && pStream->r_frame_rate.den != 0) {
        fps = float(av_q2d(pStream->r_frame_rate));
    }
#endif
    if (fps == 0) { 
        float duration = float(pStream->duration)*float(av_q2d(pStream->time_base));
        fps = pStream->nb_frames/duration;
    }
    AVG_ASSERT(fps < 10000);
/*
    cerr << "getStreamFPS: fps= " << fps << endl;
    cerr << "    r_frame_rate num: " << m_pVStream->r_frame_rate.num << ", den: " << m_pVStream->r_frame_rate.den << endl;
    cerr << "    avg_frame_rate: num: " << m_pVStream->avg_frame_rate.num << ", den: " << m_pVStream->avg_frame_rate.den << endl;
    cerr << "    numFrames= " << getNumFrames() << ", duration= " 
                << getDuration(SS_VIDEO) << endl;
*/
    return fps;
}

}
