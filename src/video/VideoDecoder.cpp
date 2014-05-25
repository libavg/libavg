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

#include "VideoDecoder.h"
#ifdef AVG_ENABLE_VDPAU
#include "VDPAUDecoder.h"
#endif

#include "../base/Exception.h"
#include "../base/Logger.h"
#include "../base/ObjectCounter.h"
#include "../base/StringHelper.h"

#include "../graphics/Bitmap.h"
#include "../graphics/BitmapLoader.h"

#include <string>

#include "WrapFFMpeg.h"

using namespace std;
using namespace boost;

namespace avg {

bool VideoDecoder::s_bInitialized = false;
boost::mutex VideoDecoder::s_OpenMutex;


VideoDecoder::VideoDecoder()
    : m_State(CLOSED),
      m_pFormatContext(0),
      m_VStreamIndex(-1),
      m_pVStream(0),
      m_PF(NO_PIXELFORMAT),
      m_Size(0,0),
#ifdef AVG_ENABLE_VDPAU
      m_pVDPAUDecoder(0),
#endif
      m_AStreamIndex(-1),
      m_pAStream(0)
{
    ObjectCounter::get()->incRef(&typeid(*this));
    initVideoSupport();
}

VideoDecoder::~VideoDecoder()
{
    if (m_pFormatContext) {
        close();
    }
#ifdef AVG_ENABLE_VDPAU
    if (m_pVDPAUDecoder) {
        delete m_pVDPAUDecoder;
    }
#endif
    ObjectCounter::get()->decRef(&typeid(*this));
}

void VideoDecoder::open(const string& sFilename, bool bUseHardwareAcceleration, 
        bool bEnableSound)
{
    lock_guard lock(s_OpenMutex);
    int err;
    m_sFilename = sFilename;
    
    AVG_TRACE(Logger::category::MEMORY, Logger::severity::INFO, "Opening " << sFilename);
    err = avformat_open_input(&m_pFormatContext, sFilename.c_str(), 0, 0);
    if (err < 0) {
        m_sFilename = "";
        m_pFormatContext = 0;
        avcodecError(sFilename, err);
    }

    err = avformat_find_stream_info(m_pFormatContext, 0);

    if (err < 0) {
        m_sFilename = "";
        m_pFormatContext = 0;
        throw Exception(AVG_ERR_VIDEO_INIT_FAILED, 
                sFilename + ": Could not find codec parameters.");
    }
    if (strcmp(m_pFormatContext->iformat->name, "image2") == 0) {
        m_sFilename = "";
        m_pFormatContext = 0;
        throw Exception(AVG_ERR_VIDEO_INIT_FAILED, 
                sFilename + ": Image files not supported as videos.");
    }
    av_read_play(m_pFormatContext);
    
    // Find audio and video streams in the file
    m_VStreamIndex = -1;
    m_AStreamIndex = -1;
    for (unsigned i = 0; i < m_pFormatContext->nb_streams; i++) {
        AVCodecContext* pContext = m_pFormatContext->streams[i]->codec;
        switch (pContext->codec_type) {
            case AVMEDIA_TYPE_VIDEO:
                if (m_VStreamIndex < 0) {
                    m_VStreamIndex = i;
                }
                break;
            case AVMEDIA_TYPE_AUDIO:
                if (m_AStreamIndex < 0 && bEnableSound) {
                    m_AStreamIndex = i;
                }
                break;
            default:
                break;
        }
    }
    
    // Enable video stream demuxing
    if (m_VStreamIndex >= 0) {
        m_pVStream = m_pFormatContext->streams[m_VStreamIndex];
        
        m_Size = IntPoint(m_pVStream->codec->width, m_pVStream->codec->height);

        char szCodec[256];
        avcodec_string(szCodec, sizeof(szCodec), m_pVStream->codec, 0);
        int rc = openCodec(m_VStreamIndex, bUseHardwareAcceleration);
        if (rc == -1) {
            m_VStreamIndex = -1;
            m_pVStream = 0;
            throw Exception(AVG_ERR_VIDEO_INIT_FAILED, 
                    sFilename + ": unsupported video codec ("+szCodec+").");
        }
        m_PF = calcPixelFormat(true);
    }
    // Enable audio stream demuxing.
    if (m_AStreamIndex >= 0) {
        m_pAStream = m_pFormatContext->streams[m_AStreamIndex];
        char szCodec[256];
        avcodec_string(szCodec, sizeof(szCodec), m_pAStream->codec, 0);
        int rc = openCodec(m_AStreamIndex, false);
        if (rc == -1) {
            m_AStreamIndex = -1;
            m_pAStream = 0; 
            throw Exception(AVG_ERR_VIDEO_INIT_FAILED, 
                    sFilename + ": unsupported audio codec ("+szCodec+").");
        }
    }
    if (!m_pVStream && !m_pAStream) {
        throw Exception(AVG_ERR_VIDEO_INIT_FAILED, 
                sFilename + ": no usable streams found.");
    }

    m_State = OPENED;
}

void VideoDecoder::startDecoding(bool bDeliverYCbCr, const AudioParams* pAP)
{
    AVG_ASSERT(m_State == OPENED);
    if (m_VStreamIndex >= 0) {
        m_PF = calcPixelFormat(bDeliverYCbCr);
    }
    bool bAudioEnabled = (pAP!=0);
    if (!bAudioEnabled) {
        m_AStreamIndex = -1;
        if (m_pAStream) {
            avcodec_close(m_pAStream->codec);
        }
        m_pAStream = 0;
    }

    if (m_AStreamIndex >= 0) {
        if (m_pAStream->codec->channels > pAP->m_Channels) {
            throw Exception(AVG_ERR_VIDEO_INIT_FAILED, 
                    m_sFilename + ": unsupported number of audio channels (" + 
                            toString(m_pAStream->codec->channels) + ").");
            m_AStreamIndex = -1;
            m_pAStream = 0; 
        }
    }

    if (!m_pVStream && !m_pAStream) {
        throw Exception(AVG_ERR_VIDEO_INIT_FAILED, 
                m_sFilename + ": no usable streams found.");
    }

    m_State = DECODING;
}

void VideoDecoder::close() 
{
    lock_guard lock(s_OpenMutex);
    AVG_TRACE(Logger::category::MEMORY, Logger::severity::INFO, "Closing " << m_sFilename);
    
    // Close audio and video codecs
    if (m_pVStream) {
        avcodec_close(m_pVStream->codec);
        m_pVStream = 0;
        m_VStreamIndex = -1;
    }

    if (m_pAStream) {
        avcodec_close(m_pAStream->codec);
        m_pAStream = 0;
        m_AStreamIndex = -1;
    }
    if (m_pFormatContext) {
#if LIBAVCODEC_VERSION_INT > AV_VERSION_INT(53, 21, 0)
        avformat_close_input(&m_pFormatContext);
#else
        av_close_input_file(m_pFormatContext);
        m_pFormatContext = 0;
#endif
    }
    
    m_State = CLOSED;
}

VideoDecoder::DecoderState VideoDecoder::getState() const
{
    return m_State;
}

VideoInfo VideoDecoder::getVideoInfo() const
{
    AVG_ASSERT(m_State != CLOSED);
    AVG_ASSERT(m_pVStream || m_pAStream);
    float duration = getDuration(SS_DEFAULT);
    
    VideoInfo info(m_pFormatContext->iformat->name, duration, m_pFormatContext->bit_rate,
            m_pVStream != 0, m_pAStream != 0);
    if (m_pVStream) {
        info.setVideoData(m_Size, getStreamPF(), getNumFrames(), getStreamFPS(),
                m_pVStream->codec->codec->name, usesVDPAU(), getDuration(SS_VIDEO));
    }
    if (m_pAStream) {
        AVCodecContext * pACodec = m_pAStream->codec;
        info.setAudioData(pACodec->codec->name, pACodec->sample_rate,
                pACodec->channels, getDuration(SS_AUDIO));
    }
    return info;
}

PixelFormat VideoDecoder::getPixelFormat() const
{
    AVG_ASSERT(m_State != CLOSED);
    return m_PF;
}

IntPoint VideoDecoder::getSize() const
{
    AVG_ASSERT(m_State != CLOSED);
    return m_Size;
}

float VideoDecoder::getStreamFPS() const
{
    AVG_ASSERT(m_State != CLOSED);
    return avg::getStreamFPS(m_pVStream);
}

FrameAvailableCode VideoDecoder::getRenderedBmp(BitmapPtr& pBmp, float timeWanted)
{
    std::vector<BitmapPtr> pBmps;
    pBmps.push_back(pBmp);
    FrameAvailableCode fa = getRenderedBmps(pBmps, timeWanted);
    pBmp = pBmps[0];
    return fa;
}

void VideoDecoder::logConfig()
{
    bool bVDPAUAvailable = false;
#ifdef AVG_ENABLE_VDPAU
    bVDPAUAvailable = VDPAUDecoder::isAvailable();
#endif
    if (bVDPAUAvailable) {
        AVG_TRACE(Logger::category::CONFIG, Logger::severity::INFO,
                "Hardware video acceleration: VDPAU");
    } else {
        AVG_TRACE(Logger::category::CONFIG, Logger::severity::INFO,
                "Hardware video acceleration: Off");
    }
}

int VideoDecoder::getNumFrames() const
{
    AVG_ASSERT(m_State != CLOSED);
    int numFrames =  int(m_pVStream->nb_frames);
    if (numFrames > 0) {
        return numFrames;
    } else {
        return int(getDuration(SS_VIDEO) * getStreamFPS());
    }
}

AVFormatContext* VideoDecoder::getFormatContext()
{
    AVG_ASSERT(m_pFormatContext);
    return m_pFormatContext;
}

bool VideoDecoder::usesVDPAU() const
{
#ifdef AVG_ENABLE_VDPAU
    AVCodecContext const* pContext = getCodecContext();
    return pContext->codec && (pContext->codec->capabilities & CODEC_CAP_HWACCEL_VDPAU);
#else
    return false;
#endif
}

AVCodecContext const* VideoDecoder::getCodecContext() const
{
    return m_pVStream->codec;
}

AVCodecContext* VideoDecoder::getCodecContext()
{
    return m_pVStream->codec;
}

void VideoDecoder::allocFrameBmps(vector<BitmapPtr>& pBmps)
{
    if (pixelFormatIsPlanar(getPixelFormat())) {
        IntPoint size = getSize();
        pBmps[0] = BitmapPtr(new Bitmap(size, I8));
        IntPoint halfSize(size.x/2, size.y/2);
        pBmps[1] = BitmapPtr(new Bitmap(halfSize, I8));
        pBmps[2] = BitmapPtr(new Bitmap(halfSize, I8));
        if (pixelFormatHasAlpha(getPixelFormat())) {
            pBmps[3] = BitmapPtr(new Bitmap(size, I8));
        }
    } else {
        pBmps[0] = BitmapPtr(new Bitmap(getSize(), getPixelFormat()));
    }
}

int VideoDecoder::getVStreamIndex() const
{
    return m_VStreamIndex;
}

AVStream* VideoDecoder::getVideoStream() const
{
    return m_pVStream;
}

int VideoDecoder::getAStreamIndex() const
{
    return m_AStreamIndex;
}

AVStream* VideoDecoder::getAudioStream() const
{
    return m_pAStream;
}

void VideoDecoder::initVideoSupport()
{
    if (!s_bInitialized) {
        av_register_all();
        s_bInitialized = true;
        // Tune libavcodec console spam.
//        av_log_set_level(AV_LOG_DEBUG);
        av_log_set_level(AV_LOG_QUIET);
    }
}

int VideoDecoder::openCodec(int streamIndex, bool bUseHardwareAcceleration)
{
    AVCodecContext* pContext;
    pContext = m_pFormatContext->streams[streamIndex]->codec;
//    pContext->debug = 0x0001; // see avcodec.h

    AVCodec * pCodec = 0;
#ifdef AVG_ENABLE_VDPAU
    if (bUseHardwareAcceleration) {
        m_pVDPAUDecoder = new VDPAUDecoder();
        pContext->opaque = m_pVDPAUDecoder;
        pCodec = m_pVDPAUDecoder->openCodec(pContext);
    } 
#endif
    if (!pCodec) {
        pCodec = avcodec_find_decoder(pContext->codec_id);
    }
    if (!pCodec) {
        return -1;
    }
    int rc = avcodec_open2(pContext, pCodec, 0);

    if (rc < 0) {
        return -1;
    }
    return 0;
}

float VideoDecoder::getDuration(StreamSelect streamSelect) const
{
    AVG_ASSERT(m_State != CLOSED);
    long long duration;
    AVRational time_base;
    if (streamSelect == SS_DEFAULT) {
        if (m_pVStream) {
            streamSelect = SS_VIDEO;
        } else {
            streamSelect = SS_AUDIO;
        }
    }
    if (streamSelect == SS_VIDEO) {
        duration = m_pVStream->duration;
        time_base = m_pVStream->time_base;
    } else {
        duration = m_pAStream->duration;
        time_base = m_pAStream->time_base;
    }
    if (duration == (long long)AV_NOPTS_VALUE) {
        return 0;
    } else {
        return float(duration)*float(av_q2d(time_base));
    }
}

PixelFormat VideoDecoder::calcPixelFormat(bool bUseYCbCr)
{
    AVCodecContext const* pContext = getCodecContext();
    if (bUseYCbCr) {
        switch(pContext->pix_fmt) {
            case PIX_FMT_YUV420P:
#ifdef AVG_ENABLE_VDPAU
            case PIX_FMT_VDPAU_H264:
            case PIX_FMT_VDPAU_MPEG1:
            case PIX_FMT_VDPAU_MPEG2:
            case PIX_FMT_VDPAU_WMV3:
            case PIX_FMT_VDPAU_VC1:
#endif
                return YCbCr420p;
            case PIX_FMT_YUVJ420P:
                return YCbCrJ420p;
            case PIX_FMT_YUVA420P:
                return YCbCrA420p;
            default:
                break;
        }
    }
    bool bAlpha = (pContext->pix_fmt == PIX_FMT_BGRA ||
            pContext->pix_fmt == PIX_FMT_YUVA420P);
    return BitmapLoader::get()->getDefaultPixelFormat(bAlpha);
}

string VideoDecoder::getStreamPF() const
{
    AVCodecContext const* pCodec = getCodecContext();
    AVPixelFormat pf = pCodec->pix_fmt;
    const char* psz = av_get_pix_fmt_name(pf);
    string s;
    if (psz) {
        s = psz;
    }
    return s;
}

void avcodecError(const string& sFilename, int err)
{
    char buf[256];
    av_strerror(err, buf, 256);
    throw Exception(AVG_ERR_VIDEO_INIT_FAILED, sFilename + ": " + buf);
}

}


