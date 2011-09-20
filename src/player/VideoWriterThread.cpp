
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

#include "VideoWriterThread.h"

#include "../base/ProfilingZone.h"
#include "../base/ScopeTimer.h"

using namespace std;

namespace avg {

const unsigned int VIDEO_BUFFER_SIZE = 400000;
const ::PixelFormat STREAM_PIXEL_FORMAT = ::PIX_FMT_YUVJ420P;

VideoWriterThread::VideoWriterThread(CQueue& CmdQueue, const string& sFilename,
        IntPoint size, int frameRate, int qMin, int qMax)
    : WorkerThread<VideoWriterThread>(sFilename, CmdQueue, Logger::PROFILE),
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

static ProfilingZoneID ProfilingZoneEncodeFrame("Encode frame");

void VideoWriterThread::encodeFrame(BitmapPtr pBmp)
{
    ScopeTimer timer(ProfilingZoneEncodeFrame);
    convertImage(pBmp);
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
            url_fclose(m_pOutputFormatContext->pb);
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
    m_pOutputFormat = av_guess_format("mov", NULL, NULL);
#else
    m_pOutputFormat = guess_format("mov", NULL, NULL);
#endif
    m_pOutputFormat->video_codec = CODEC_ID_MJPEG;

#if LIBAVFORMAT_VERSION_INT >= AV_VERSION_INT(52, 24, 0)
    m_pOutputFormatContext = avformat_alloc_context();
#else
    m_pOutputFormatContext = av_alloc_format_context();
#endif
    m_pOutputFormatContext->oformat = m_pOutputFormat;

    strncpy(m_pOutputFormatContext->filename, m_sFilename.c_str(),
            sizeof(m_pOutputFormatContext->filename));

    if (m_pOutputFormat->video_codec != CODEC_ID_NONE) {
        setupVideoStream();
    }
#if LIBAVFORMAT_VERSION_MAJOR < 52
    av_set_parameters(m_pOutputFormatContext, NULL);
#endif

    double muxPreload = 0.5;
    double muxMaxDelay = 0.7;
    m_pOutputFormatContext->preload = int(muxPreload * AV_TIME_BASE);
    m_pOutputFormatContext->max_delay = int(muxMaxDelay * AV_TIME_BASE);

//    av_dump_format(m_pOutputFormatContext, 0, m_sFilename.c_str(), 1);

    openVideoCodec();

    m_pVideoBuffer = NULL;
    if (!(m_pOutputFormatContext->oformat->flags & AVFMT_RAWPICTURE)) {
        m_pVideoBuffer = (unsigned char*)(av_malloc(VIDEO_BUFFER_SIZE));
    }

    if (!(m_pOutputFormat->flags & AVFMT_NOFILE)) {
        int retVal = url_fopen(&m_pOutputFormatContext->pb, m_sFilename.c_str(),
                URL_WRONLY);
        if (retVal < 0) {
            throw Exception(AVG_ERR_VIDEO_INIT_FAILED, 
                    string("Could not open output file: '") + m_sFilename + "'");
        }
    }

    m_pFrameConversionContext = sws_getContext(m_Size.x, m_Size.y, 
            ::PIX_FMT_RGB32, m_Size.x, m_Size.y, STREAM_PIXEL_FORMAT, 
            SWS_FAST_BILINEAR, NULL, NULL, NULL);

    m_pConvertedFrame = createFrame(STREAM_PIXEL_FORMAT, m_Size);

#if LIBAVFORMAT_VERSION_MAJOR > 52
    avformat_write_header(m_pOutputFormatContext, 0);
#else
    av_write_header(m_pOutputFormatContext);
#endif
}

void VideoWriterThread::setupVideoStream()
{
    m_pVideoStream = av_new_stream(m_pOutputFormatContext, 0);

    AVCodecContext* pCodecContext = m_pVideoStream->codec;
    pCodecContext->codec_id = static_cast<CodecID>(m_pOutputFormat->video_codec);
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
    pCodecContext->gop_size = 12; /* emit one intra frame every twelve frames at most */
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
    /* find the video encoder */
    AVCodec* videoCodec = avcodec_find_encoder(
            m_pVideoStream->codec->codec_id);
    if (!videoCodec) {
        cerr << "codec not found" << endl;
    }

    /* open the codec */
    if (avcodec_open(m_pVideoStream->codec, videoCodec) < 0) {
        cerr << "could not open codec" << endl;
    }
}

AVFrame* VideoWriterThread::createFrame(::PixelFormat pixelFormat, IntPoint size)
{
    AVFrame* pPicture;

    pPicture = avcodec_alloc_frame();
    if (!pPicture) {
        return NULL;
    }

    int memNeeded = avpicture_get_size(pixelFormat, size.x, size.y);
    m_pPictureBuffer = static_cast<unsigned char*>(av_malloc(memNeeded));
    if (!m_pPictureBuffer) {
        av_free(pPicture);
        return NULL;
    }
    avpicture_fill(reinterpret_cast<AVPicture*>(pPicture),
            m_pPictureBuffer, pixelFormat, size.x, size.y);

    return pPicture;
}

static ProfilingZoneID ProfilingZoneConvertImage(" Convert image");

void VideoWriterThread::convertImage(BitmapPtr pBitmap)
{
    ScopeTimer timer(ProfilingZoneConvertImage);
    unsigned char* rgbData[3] = {pBitmap->getPixels(), NULL, NULL};
    int rgbStride[3] = {pBitmap->getLineLen(), 0, 0};

    sws_scale(m_pFrameConversionContext, rgbData, rgbStride,
              0, m_Size.y, m_pConvertedFrame->data, m_pConvertedFrame->linesize);
}

static ProfilingZoneID ProfilingZoneWriteFrame(" Write frame");

void VideoWriterThread::writeFrame(AVFrame* pFrame)
{
    ScopeTimer timer(ProfilingZoneWriteFrame);
    m_FramesWritten++;
    AVCodecContext* pCodecContext = m_pVideoStream->codec;
    int out_size = avcodec_encode_video(pCodecContext, m_pVideoBuffer,
            VIDEO_BUFFER_SIZE, pFrame);

    /* if zero size, it means the image was buffered */
    if (out_size > 0) {
        AVPacket packet;
        av_init_packet(&packet);

        if ((unsigned long long)(pCodecContext->coded_frame->pts) != AV_NOPTS_VALUE) {
            packet.pts = av_rescale_q(pCodecContext->coded_frame->pts,
                    pCodecContext->time_base, m_pVideoStream->time_base);
        }

        if (pCodecContext->coded_frame->key_frame) {
#if LIBAVCODEC_VERSION_INT > AV_VERSION_INT(52, 31, 0)
            packet.flags |= AV_PKT_FLAG_KEY;
#else
            packet.flags |= PKT_FLAG_KEY;
#endif
        }
        packet.stream_index = m_pVideoStream->index;
        packet.data = m_pVideoBuffer;
        packet.size = out_size;

        /* write the compressed frame in the media file */
        int ret = av_interleaved_write_frame(m_pOutputFormatContext, &packet);
        if (ret != 0) {
            std::cerr << "Error while writing video frame" << std::endl;
            assert(false);
        }
    }

}

}

