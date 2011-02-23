
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

#include "VideoWriterThread.h"

using namespace std;

namespace avg {

const unsigned int VIDEO_BUFFER_SIZE = 400000;
const ffmpeg::PixelFormat STREAM_PIXEL_FORMAT = ffmpeg::PIX_FMT_YUVJ420P;

VideoWriterThread::VideoWriterThread(CQueue& CmdQueue, const string& sFilename,
        IntPoint size, int frameRate, int qMin, int qMax)
    : WorkerThread<VideoWriterThread>(sFilename, CmdQueue),
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

void VideoWriterThread::encodeFrame(BitmapPtr pBmp)
{
    convertImage(pBmp);
    writeFrame(m_pConvertedFrame);
}

void VideoWriterThread::close()
{
    if (m_pOutputFormatContext) {
        ffmpeg::av_write_trailer(m_pOutputFormatContext);
        ffmpeg::avcodec_close(m_pVideoStream->codec);

        for (unsigned int i=0; i<m_pOutputFormatContext->nb_streams; i++) {
            ffmpeg::AVStream* pStream = m_pOutputFormatContext->streams[i];

            pStream->discard = ffmpeg::AVDISCARD_ALL;
            ffmpeg::av_freep(&m_pOutputFormatContext->streams[i]->codec);
            ffmpeg::av_freep(&m_pOutputFormatContext->streams[i]);
        }

        if (!(m_pOutputFormat->flags & AVFMT_NOFILE)) {
            ffmpeg::url_fclose(m_pOutputFormatContext->pb);
        }

        ffmpeg::av_free(m_pOutputFormatContext);
        ffmpeg::av_free(m_pVideoBuffer);
        ffmpeg::av_free(m_pConvertedFrame);
        ffmpeg::av_free(m_pPictureBuffer);
        ffmpeg::sws_freeContext(m_pFrameConversionContext);
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
    return true;
}

void VideoWriterThread::deinit()
{
    close();
}

void VideoWriterThread::open()
{
    ffmpeg::av_register_all(); // TODO: make sure this is only done once. 
//    ffmpeg::av_log_set_level(AV_LOG_DEBUG);
    m_pOutputFormat = ffmpeg::guess_format("mov", NULL, NULL);
    m_pOutputFormat->video_codec = ffmpeg::CODEC_ID_MJPEG;

    m_pOutputFormatContext = ffmpeg::av_alloc_format_context();

    m_pOutputFormatContext->oformat = m_pOutputFormat;

    strncpy(m_pOutputFormatContext->filename, m_sFilename.c_str(),
            sizeof(m_pOutputFormatContext->filename));

    if (m_pOutputFormat->video_codec != ffmpeg::CODEC_ID_NONE) {
        setupVideoStream();
    }

    av_set_parameters(m_pOutputFormatContext, NULL);

    double muxPreload = 0.5;
    double muxMaxDelay = 0.7;
    m_pOutputFormatContext->preload = int(muxPreload * AV_TIME_BASE);
    m_pOutputFormatContext->max_delay = int(muxMaxDelay * AV_TIME_BASE);

    ffmpeg::dump_format(m_pOutputFormatContext, 0, m_sFilename.c_str(), 1);

    openVideoCodec();

    m_pVideoBuffer = NULL;
    if (!(m_pOutputFormatContext->oformat->flags & AVFMT_RAWPICTURE)) {
        /* allocate output buffer */
        /* XXX: API change will be done */
        /* buffers passed into lav* can be allocated any way you prefer,
           as long as they're aligned enough for the architecture, and
           they're freed appropriately (such as using av_free for buffers
           allocated with av_malloc) */
        m_pVideoBuffer = (unsigned char*)(ffmpeg::av_malloc(VIDEO_BUFFER_SIZE));
    }

    if (!(m_pOutputFormat->flags & AVFMT_NOFILE)) {
        int retVal = url_fopen(&m_pOutputFormatContext->pb, m_sFilename.c_str(),
                URL_WRONLY);
        if (retVal < 0) {
            // TODO: Throw exception
            cerr << "Could not open output file: " <<  m_sFilename << endl;
        }
    }

    m_pFrameConversionContext = ffmpeg::sws_getContext(m_Size.x, m_Size.y, 
            ffmpeg::PIX_FMT_BGRA, m_Size.x, m_Size.y, STREAM_PIXEL_FORMAT, 
            SWS_BICUBIC, NULL, NULL, NULL);

    m_pConvertedFrame = createFrame(STREAM_PIXEL_FORMAT, m_Size);

    ffmpeg::av_write_header(m_pOutputFormatContext);
}

void VideoWriterThread::setupVideoStream()
{
    m_pVideoStream = ffmpeg::av_new_stream(m_pOutputFormatContext, 0);

    ffmpeg::AVCodecContext* pCodecContext = m_pVideoStream->codec;
    pCodecContext->codec_id = static_cast<ffmpeg::CodecID>(m_pOutputFormat->video_codec);
    pCodecContext->codec_type = ffmpeg::CODEC_TYPE_VIDEO;

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
}

void VideoWriterThread::openVideoCodec()
{
    /* find the video encoder */
    ffmpeg::AVCodec* videoCodec = ffmpeg::avcodec_find_encoder(
            m_pVideoStream->codec->codec_id);
    if (!videoCodec) {
        cerr << "codec not found" << endl;
    }

    /* open the codec */
    if (ffmpeg::avcodec_open(m_pVideoStream->codec, videoCodec) < 0) {
        cerr << "could not open codec" << endl;
    }
}

ffmpeg::AVFrame* VideoWriterThread::createFrame(ffmpeg::PixelFormat pixelFormat,
        IntPoint size)
{
    ffmpeg::AVFrame* pPicture;

    pPicture = ffmpeg::avcodec_alloc_frame();
    if (!pPicture) {
        return NULL;
    }

    int memNeeded = ffmpeg::avpicture_get_size(pixelFormat, size.x, size.y);
    m_pPictureBuffer = static_cast<unsigned char*>(ffmpeg::av_malloc(memNeeded));
    if (!m_pPictureBuffer) {
        ffmpeg::av_free(pPicture);
        return NULL;
    }
    ffmpeg::avpicture_fill(reinterpret_cast<ffmpeg::AVPicture*>(pPicture),
            m_pPictureBuffer, pixelFormat, size.x, size.y);

    return pPicture;
}

void VideoWriterThread::convertImage(BitmapPtr pBitmap)
{
    unsigned char* rgbData[3] = {pBitmap->getPixels(), NULL, NULL};
    int rgbStride[3] = {pBitmap->getLineLen(), 0, 0};

    sws_scale(m_pFrameConversionContext, rgbData, rgbStride,
              0, m_Size.y, m_pConvertedFrame->data, m_pConvertedFrame->linesize);
}

void VideoWriterThread::writeFrame(ffmpeg::AVFrame* pFrame)
{
    ffmpeg::AVCodecContext* pCodecContext = m_pVideoStream->codec;
    int out_size = ffmpeg::avcodec_encode_video(pCodecContext, m_pVideoBuffer,
            VIDEO_BUFFER_SIZE, pFrame);

    /* if zero size, it means the image was buffered */
    if (out_size > 0) {
        ffmpeg::AVPacket packet;
        ffmpeg::av_init_packet(&packet);

        if (pCodecContext->coded_frame->pts != AV_NOPTS_VALUE) {
            packet.pts = av_rescale_q(pCodecContext->coded_frame->pts,
                    pCodecContext->time_base, m_pVideoStream->time_base);
        }

        if (pCodecContext->coded_frame->key_frame) {
            packet.flags |= PKT_FLAG_KEY;
        }
        packet.stream_index = m_pVideoStream->index;
        packet.data = m_pVideoBuffer;
        packet.size = out_size;

        /* write the compressed frame in the media file */
        int ret = ffmpeg::av_interleaved_write_frame(m_pOutputFormatContext, &packet);
        if (ret != 0) {
            std::cerr << "Error while writing video frame" << std::endl;
            assert(false);
        }
    }

}

}

