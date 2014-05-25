
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

#include "VideoWriterThread.h"

#include "../base/ProfilingZoneID.h"
#include "../base/ScopeTimer.h"
#include "../base/StringHelper.h"

#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(55, 18, 102)
    typedef CodecID AVCodecID;
#endif

using namespace std;

namespace avg {

const unsigned int VIDEO_BUFFER_SIZE = 400000;
const AVPixelFormat STREAM_PIXEL_FORMAT = ::PIX_FMT_YUVJ420P;

VideoWriterThread::VideoWriterThread(CQueue& cmdQueue, const string& sFilename,
        IntPoint size, int frameRate, int qMin, int qMax)
    : WorkerThread<VideoWriterThread>(sFilename, cmdQueue, Logger::category::PROFILE),
      m_sFilename(sFilename),
      m_Size(size),
      m_FrameRate(frameRate),
      m_QMin(qMin),
      m_QMax(qMax),
      m_pOutputFormatContext()
{
}

VideoWriterThread::~VideoWriterThread()
{
}

static ProfilingZoneID ProfilingZoneEncodeFrame("Encode frame", true);

void VideoWriterThread::encodeYUVFrame(BitmapPtr pBmp)
{
    ScopeTimer timer(ProfilingZoneEncodeFrame);
    convertYUVImage(pBmp);
    writeFrame(m_pConvertedFrame);
    ThreadProfiler::get()->reset();
}

void VideoWriterThread::encodeFrame(BitmapPtr pBmp)
{
    ScopeTimer timer(ProfilingZoneEncodeFrame);
    convertRGBImage(pBmp);
    writeFrame(m_pConvertedFrame);
    ThreadProfiler::get()->reset();
}

void VideoWriterThread::close()
{
    if (m_pOutputFormatContext) {
        av_write_trailer(m_pOutputFormatContext);
        avcodec_close(m_pVideoStream->codec);

        for (unsigned int i=0; i<m_pOutputFormatContext->nb_streams; i++) {
            AVStream* pStream = m_pOutputFormatContext->streams[i];

            pStream->discard = AVDISCARD_ALL;
            av_freep(&m_pOutputFormatContext->streams[i]->codec);
            av_freep(&m_pOutputFormatContext->streams[i]);
        }

        if (!(m_pOutputFormat->flags & AVFMT_NOFILE)) {
#if LIBAVCODEC_VERSION_INT > AV_VERSION_INT(53, 8, 0)
            avio_close(m_pOutputFormatContext->pb);
#else            
            url_fclose(m_pOutputFormatContext->pb);
#endif
        }

        av_free(m_pOutputFormatContext);
        av_free(m_pVideoBuffer);
        av_free(m_pConvertedFrame);
        av_free(m_pPictureBuffer);
        sws_freeContext(m_pFrameConversionContext);
        m_pOutputFormatContext = 0;
    }
}

bool VideoWriterThread::init()
{
    open();
    return true;
}

bool VideoWriterThread::work()
{
    waitForCommand();
    return true;
}

void VideoWriterThread::deinit()
{
    close();
}

void VideoWriterThread::open()
{
    av_register_all(); // TODO: make sure this is only done once. 
//    av_log_set_level(AV_LOG_DEBUG);
#if LIBAVFORMAT_VERSION_MAJOR > 52
    m_pOutputFormat = av_guess_format(0, m_sFilename.c_str(), 0);
#else
    m_pOutputFormat = guess_format(0, m_sFilename.c_str(), 0);
#endif
    m_pOutputFormat->video_codec = AV_CODEC_ID_MJPEG;

#if LIBAVFORMAT_VERSION_INT >= AV_VERSION_INT(52, 24, 0)
    m_pOutputFormatContext = avformat_alloc_context();
#else
    m_pOutputFormatContext = av_alloc_format_context();
#endif
    m_pOutputFormatContext->oformat = m_pOutputFormat;

    strncpy(m_pOutputFormatContext->filename, m_sFilename.c_str(),
            sizeof(m_pOutputFormatContext->filename));

    if (m_pOutputFormat->video_codec != AV_CODEC_ID_NONE) {
        setupVideoStream();
    }
#if LIBAVFORMAT_VERSION_MAJOR < 52
    av_set_parameters(m_pOutputFormatContext, NULL);
#endif

    float muxMaxDelay = 0.7;
    m_pOutputFormatContext->max_delay = int(muxMaxDelay * AV_TIME_BASE);

//    av_dump_format(m_pOutputFormatContext, 0, m_sFilename.c_str(), 1);

    openVideoCodec();

    m_pVideoBuffer = NULL;
    if (!(m_pOutputFormatContext->oformat->flags & AVFMT_RAWPICTURE)) {
        m_pVideoBuffer = (unsigned char*)(av_malloc(VIDEO_BUFFER_SIZE));
    }

    if (!(m_pOutputFormat->flags & AVFMT_NOFILE)) {
#if LIBAVCODEC_VERSION_INT > AV_VERSION_INT(53, 8, 0)
        int retVal = avio_open(&m_pOutputFormatContext->pb, m_sFilename.c_str(),
                URL_WRONLY);
#else
        int retVal = url_fopen(&m_pOutputFormatContext->pb, m_sFilename.c_str(),
                URL_WRONLY);
#endif
        if (retVal < 0) {
            throw Exception(AVG_ERR_VIDEO_INIT_FAILED, 
                    string("Could not open output file: '") + m_sFilename + "'");
        }
    }

    m_pFrameConversionContext = sws_getContext(m_Size.x, m_Size.y, 
            ::PIX_FMT_RGB32, m_Size.x, m_Size.y, STREAM_PIXEL_FORMAT, 
            SWS_BILINEAR, NULL, NULL, NULL);

    m_pConvertedFrame = createFrame(STREAM_PIXEL_FORMAT, m_Size);

#if LIBAVFORMAT_VERSION_MAJOR > 52
    avformat_write_header(m_pOutputFormatContext, 0);
#else
    av_write_header(m_pOutputFormatContext);
#endif
}

void VideoWriterThread::setupVideoStream()
{
#if LIBAVCODEC_VERSION_INT > AV_VERSION_INT(53, 21, 0)
    m_pVideoStream = avformat_new_stream(m_pOutputFormatContext, 0);
#else
    m_pVideoStream = av_new_stream(m_pOutputFormatContext, 0);
#endif

    AVCodecContext* pCodecContext = m_pVideoStream->codec;
    pCodecContext->codec_id = static_cast<AVCodecID>(m_pOutputFormat->video_codec);
    pCodecContext->codec_type = AVMEDIA_TYPE_VIDEO;

    /* put sample parameters */
    pCodecContext->bit_rate = 400000;
    /* resolution must be a multiple of two */
    pCodecContext->width = m_Size.x;
    pCodecContext->height = m_Size.y;
    /* time base: this is the fundamental unit of time (in seconds) in terms
       of which frame timestamps are represented. for fixed-fps content,
       timebase should be 1/framerate and timestamp increments should be
       identically 1. */
    pCodecContext->time_base.den = m_FrameRate;
    pCodecContext->time_base.num = 1;
//    pCodecContext->gop_size = 12; /* emit one intra frame every twelve frames at most */
    pCodecContext->pix_fmt = STREAM_PIXEL_FORMAT;
    // Quality of quantization
    pCodecContext->qmin = m_QMin;
    pCodecContext->qmax = m_QMax;
    // some formats want stream headers to be separate
    if (m_pOutputFormatContext->oformat->flags & AVFMT_GLOBALHEADER) {
        pCodecContext->flags |= CODEC_FLAG_GLOBAL_HEADER;
    }
    m_FramesWritten = 0;
}

void VideoWriterThread::openVideoCodec()
{
    AVCodec* videoCodec = avcodec_find_encoder(m_pVideoStream->codec->codec_id);
    AVG_ASSERT(videoCodec);

    int rc = avcodec_open2(m_pVideoStream->codec, videoCodec, 0);
    AVG_ASSERT(rc == 0);
}

AVFrame* VideoWriterThread::createFrame(AVPixelFormat pixelFormat, IntPoint size)
{
    AVFrame* pPicture;

    pPicture = avcodec_alloc_frame();

    int memNeeded = avpicture_get_size(pixelFormat, size.x, size.y);
    m_pPictureBuffer = static_cast<unsigned char*>(av_malloc(memNeeded));
    avpicture_fill(reinterpret_cast<AVPicture*>(pPicture),
            m_pPictureBuffer, pixelFormat, size.x, size.y);

    return pPicture;
}

static ProfilingZoneID ProfilingZoneConvertImage(" Convert image", true);

void VideoWriterThread::convertRGBImage(BitmapPtr pSrcBmp)
{
    ScopeTimer timer(ProfilingZoneConvertImage);
    unsigned char* rgbData[3] = {pSrcBmp->getPixels(), NULL, NULL};
    int rgbStride[3] = {pSrcBmp->getLineLen(), 0, 0};

    sws_scale(m_pFrameConversionContext, rgbData, rgbStride,
              0, m_Size.y, m_pConvertedFrame->data, m_pConvertedFrame->linesize);
}

void VideoWriterThread::convertYUVImage(BitmapPtr pSrcBmp)
{
    ScopeTimer timer(ProfilingZoneConvertImage);
    IntPoint size = pSrcBmp->getSize();
    BitmapPtr pYBmp(new Bitmap(size, I8, m_pConvertedFrame->data[0], 
            m_pConvertedFrame->linesize[0], false));
    BitmapPtr pUBmp(new Bitmap(size/2, I8, m_pConvertedFrame->data[1], 
            m_pConvertedFrame->linesize[1], false));
    BitmapPtr pVBmp(new Bitmap(size/2, I8, m_pConvertedFrame->data[2], 
            m_pConvertedFrame->linesize[2], false));
    for (int y=0; y<size.y/2; ++y) {
        int srcStride = pSrcBmp->getStride();
        const unsigned char * pSrc = pSrcBmp->getPixels() + y*srcStride*2;
        int yStride = pYBmp->getStride();
        unsigned char * pYDest = pYBmp->getPixels() + y*yStride*2;
        unsigned char * pUDest = pUBmp->getPixels() + y*pUBmp->getStride();
        unsigned char * pVDest = pVBmp->getPixels() + y*pVBmp->getStride();
        for (int x=0; x<size.x/2; ++x) {
            *pYDest = *pSrc;
            *(pYDest+1) = *(pSrc+4);
            *(pYDest+yStride) = *(pSrc+srcStride);
            *(pYDest+yStride+1) = *(pSrc+srcStride+4);

            *pUDest = ((int)*(pSrc+1) + *(pSrc+5) + 
                       *(pSrc+srcStride+1) + *(pSrc+srcStride+5) + 2)/4; 

            *pVDest = ((int)*(pSrc+2) + *(pSrc+6) + 
                       *(pSrc+srcStride+2) + *(pSrc+srcStride+6) + 2)/4; 

            pSrc += 8;
            pYDest += 2;
            pUDest += 1;
            pVDest += 1;
        }
    }
//    pSrcBmp->save("src"+toString(m_FramesWritten)+".png");
//    pUBmp->save("foo"+toString(m_FramesWritten)+".png");
}

static ProfilingZoneID ProfilingZoneWriteFrame(" Write frame", true);

void VideoWriterThread::writeFrame(AVFrame* pFrame)
{
    ScopeTimer timer(ProfilingZoneWriteFrame);
    m_FramesWritten++;
    AVCodecContext* pCodecContext = m_pVideoStream->codec;
    AVPacket packet = { 0 };
    int ret, out_size = 0;

    #if LIBAVCODEC_VERSION_INT > AV_VERSION_INT(54, 0, 0)
        int got_output = 0;
        ret = avcodec_encode_video2(pCodecContext, &packet, pFrame, &got_output);
        if(ret < 0) {
            av_free_packet(&packet);
            AVG_ASSERT(false);
        }
        out_size = packet.size;
    #else
        out_size = avcodec_encode_video(pCodecContext, m_pVideoBuffer,
                VIDEO_BUFFER_SIZE, pFrame);
        if(out_size > 0) {
            av_init_packet(&packet);

            if ((pCodecContext->coded_frame->pts) != (long long)AV_NOPTS_VALUE) {
                packet.pts = av_rescale_q(pCodecContext->coded_frame->pts,
                        pCodecContext->time_base, m_pVideoStream->time_base);
            }

            if (pCodecContext->coded_frame->key_frame) {
                packet.flags |= AV_PKT_FLAG_KEY;
            }
            packet.stream_index = m_pVideoStream->index;
            packet.data = m_pVideoBuffer;
            packet.size = out_size;
        }
    #endif

    ///* if zero size, it means the image was buffered */
    if (out_size > 0) {
        /* write the compressed frame in the media file */
        ret = av_interleaved_write_frame(m_pOutputFormatContext, &packet);
        av_free_packet(&packet);
        AVG_ASSERT(ret == 0);
    }

}

}

