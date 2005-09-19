//
// $Id$
// 

#include "FFMpegDecoder.h"

#include "IDisplayEngine.h"
#include "Player.h"
#include "../base/Exception.h"
#include "../base/Logger.h"

#include <iostream>
#include <sstream>
#include <unistd.h>

using namespace std;

namespace avg {

bool FFMpegDecoder::m_bInitialized = false;

FFMpegDecoder::FFMpegDecoder ()
    : m_pFormatContext(0),
      m_pVStream(0),
      m_pPacketData(0)
{
    initVideoSupport();
}

FFMpegDecoder::~FFMpegDecoder ()
{
    if (m_pFormatContext) {
        close();
    }
}


void avcodecError(const string & filename, int err)
{
    switch(err) {
        case AVERROR_NUMEXPECTED:
            AVG_TRACE(Logger::ERROR, 
                    filename << ": Incorrect image filename syntax.");
            AVG_TRACE(Logger::ERROR, 
                    "Use '%%d' to specify the image number:");
            AVG_TRACE(Logger::ERROR, 
                    "  for img1.jpg, img2.jpg, ..., use 'img%%d.jpg';");
            AVG_TRACE(Logger::ERROR, 
                    "  for img001.jpg, img002.jpg, ..., use 'img%%03d.jpg'."); 
            break;
        case AVERROR_INVALIDDATA:
            AVG_TRACE(Logger::ERROR, 
                    filename << ": Error while parsing header");
            break;
        case AVERROR_NOFMT:
            AVG_TRACE(Logger::ERROR, 
                    filename << ": Unknown format");
            break;
        default:
            AVG_TRACE(Logger::ERROR, 
                    filename << ": Error while opening file (Num:" << err << ")");
            break;
    }
    // TODO: Continue without video.
    exit(-1);
}


void dump_stream_info(AVFormatContext *s)
{
    cerr << "Stream info: " << endl;
    if (s->track != 0)
        fprintf(stderr, "  Track: %d\n", s->track);
    if (s->title[0] != '\0')
        fprintf(stderr, "  Title: %s\n", s->title);
    if (s->author[0] != '\0')
        fprintf(stderr, "  Author: %s\n", s->author);
    if (s->album[0] != '\0')
        fprintf(stderr, "  Album: %s\n", s->album);
    if (s->year != 0)
        fprintf(stderr, "  Year: %d\n", s->year);
    if (s->genre[0] != '\0')
        fprintf(stderr, "  Genre: %s\n", s->genre);
}


bool FFMpegDecoder::open (const std::string& sFilename, 
        int* pWidth, int* pHeight)
{
    AVFormatParameters params;
    int err;
    m_sFilename = sFilename;

    AVG_TRACE(Logger::PROFILE, "Opening " << sFilename);
    memset(&params, 0, sizeof(params));
    params.image_format = 0;

    err = av_open_input_file(&m_pFormatContext, sFilename.c_str(), 
            0, 0, &params);
    if (err < 0) {
        avcodecError(sFilename, err);
    }
    
    err = av_find_stream_info(m_pFormatContext);
    if (err < 0) {
        AVG_TRACE(Logger::ERROR, 
                    sFilename << ": Could not find codec parameters.");
        return false;
    }

    av_read_play(m_pFormatContext);
    
    m_VStreamIndex = -1;
    for(int i = 0; i < m_pFormatContext->nb_streams; i++) {
#if LIBAVFORMAT_BUILD < ((49<<16)+(0<<8)+0)
        AVCodecContext *enc = &m_pFormatContext->streams[i]->codec;
#else         
        AVCodecContext *enc = m_pFormatContext->streams[i]->codec;
#endif
        switch(enc->codec_type) {
/*
           case CODEC_TYPE_AUDIO:
               if (audio_index < 0 && !audio_disable)
               audio_index = i;
               break;
*/
            case CODEC_TYPE_VIDEO:
                if (m_VStreamIndex < 0)
                    m_VStreamIndex = i;
                break;
            default:
                break;
        }
    }
//    dump_format(m_pFormatContext, 0, m_sFilename.c_str(), 0);
//    dump_stream_info(m_pFormatContext);
    if (m_VStreamIndex < 0) {
        AVG_TRACE(Logger::ERROR, 
                    sFilename << " does not contain any video streams.");
        return false;
    }                
    AVCodecContext *enc;
#if LIBAVFORMAT_BUILD < ((49<<16)+(0<<8)+0)
    enc = &(m_pFormatContext->streams[m_VStreamIndex]->codec);
#else
    enc = m_pFormatContext->streams[m_VStreamIndex]->codec;
#endif
//    enc->debug = 0x0001; // see avcodec.h

    AVCodec * codec = avcodec_find_decoder(enc->codec_id);
    if (!codec ||
        avcodec_open(enc, codec) < 0)
    {
        AVG_TRACE(Logger::ERROR, 
                    sFilename << ": could not open codec (?!).");
        return false;
    }                
    m_pVStream = m_pFormatContext->streams[m_VStreamIndex];
#if LIBAVFORMAT_BUILD < ((49<<16)+(0<<8)+0)
    *pWidth =  m_pVStream->codec.width;
    *pHeight = m_pVStream->codec.height;
#else
    *pWidth =  m_pVStream->codec->width;
    *pHeight = m_pVStream->codec->height;
#endif
    m_bFirstPacket = true;
    m_PacketLenLeft = 0;
    m_bEOF = false;
    return true;
} 

void FFMpegDecoder::close() 
{
    AVG_TRACE(Logger::PROFILE, "Closing " << m_sFilename);
    AVCodecContext * enc;
#if LIBAVFORMAT_BUILD < ((49<<16)+(0<<8)+0)
    enc = &(m_pFormatContext->streams[m_VStreamIndex]->codec);
#else
    enc = m_pFormatContext->streams[m_VStreamIndex]->codec;
#endif
    if (!m_bFirstPacket) {
        av_free_packet(&m_Packet);
    }
    m_pPacketData = 0;
    avcodec_close(enc);
    m_pVStream = 0;
    av_close_input_file(m_pFormatContext);
    m_pFormatContext = 0;
}

void FFMpegDecoder::seek(int DestFrame, int CurFrame) 
{
#if LIBAVFORMAT_BUILD <= 4616
    av_seek_frame(m_pFormatContext, m_VStreamIndex, 
            int((double(DestFrame)*1000000*1000)/m_pVStream->r_frame_rate));
#else
#if LIBAVFORMAT_BUILD < ((49<<16)+(0<<8)+0)
    av_seek_frame(m_pFormatContext, m_VStreamIndex, 
            int((double(DestFrame)*1000000*1000)/m_pVStream->r_frame_rate), 0);
#else
    double framerate = (m_pVStream->r_frame_rate.num)/m_pVStream->r_frame_rate.den;
    av_seek_frame(m_pFormatContext, m_VStreamIndex, 
            int((double(DestFrame)*1000000*1000)/framerate), 0);
#endif
#endif    
}

int FFMpegDecoder::getNumFrames()
{
    // This is broken for some videos, but the code here is correct.
    // So fix the videos or ffmpeg :-).
#if LIBAVFORMAT_BUILD < ((49<<16)+(0<<8)+0)
    return m_pVStream->r_frame_rate*(m_pVStream->duration/AV_TIME_BASE);
#else
    return (m_pVStream->r_frame_rate.num/m_pVStream->r_frame_rate.den)*(m_pVStream->duration/AV_TIME_BASE);
#endif 
}

double FFMpegDecoder::getFPS()
{
#if LIBAVFORMAT_BUILD < ((49<<16)+(0<<8)+0)
    return m_pVStream->r_frame_rate;
#else
    return (m_pVStream->r_frame_rate.num/m_pVStream->r_frame_rate.den);
#endif 
}

bool FFMpegDecoder::renderToBmp(BitmapPtr pBmp)
{
/* Speedup possibilities:
    fast YUV->RGB conversion? incl. scaling?
    draw_horiz_band?
*/
    AVFrame Frame;
    readFrame(Frame);
    if (!m_bEOF) {
        AVPicture DestPict;
        int x1 = 0;
        int y1 = 0;
        unsigned char * pDestBits = pBmp->getPixels()+pBmp->getStride()*y1+3*x1;
        DestPict.data[0] = pDestBits;
        DestPict.data[1] = pDestBits+1;
        DestPict.data[2] = pDestBits+2;
        DestPict.linesize[0] = pBmp->getStride();
        DestPict.linesize[1] = pBmp->getStride();
        DestPict.linesize[2] = pBmp->getStride();
        int DestFmt;
        switch(pBmp->getPixelFormat()) {
            case R8G8B8:
                DestFmt = PIX_FMT_RGB24;
                break;
            case B8G8R8:
                DestFmt = PIX_FMT_BGR24;
                break;
            default:
                assert(false);
        }
        img_convert(&DestPict, DestFmt,
                (AVPicture*)&Frame, m_pVStream->codec.pix_fmt,
                m_pVStream->codec.width, m_pVStream->codec.height);
    }
    return m_bEOF;
}

bool FFMpegDecoder::canRenderToBuffer(int BPP)
{
//TODO: This is a bug: We should enable direct rendering if DFB is being 
//      used and not just compiled in!
#ifdef AVG_ENABLE_DFB
    return (BPP == 24);
#else
    return false;
#endif
}

void FFMpegDecoder::initVideoSupport()
{
    if (!m_bInitialized) {
        av_register_all();
        m_bInitialized = true;
        // Avoid libavcodec console spam - turn this up again when libavcodec
        // doesn't spam anymore :-).
        av_log_set_level(AV_LOG_QUIET);
    }
}

void FFMpegDecoder::readFrame(AVFrame& Frame)
{
    if (m_pVStream->codec.codec_id == CODEC_ID_RAWVIDEO) {
        AVPacket Packet;
        m_bEOF = getNextVideoPacket(Packet);
        if (m_bEOF) {
            return ;
        }
        avpicture_fill((AVPicture*)&Frame, Packet.data, 
                m_pVStream->codec.pix_fmt, 
                m_pVStream->codec.width, m_pVStream->codec.height);
    } else {
        int gotPicture = 0;
        while (!gotPicture) {
            if (m_PacketLenLeft <= 0) {
                if (!m_bFirstPacket) {
                    av_free_packet(&m_Packet);
                }
                m_bFirstPacket = false;
                m_bEOF = getNextVideoPacket(m_Packet);
                if (m_bEOF) {
                    return ;
                }
                m_PacketLenLeft = m_Packet.size;
                m_pPacketData = m_Packet.data;
            }
            int Len1 = avcodec_decode_video(&m_pVStream->codec, &Frame,
                    &gotPicture, m_pPacketData, m_PacketLenLeft);
            if (Len1 < 0) {
                AVG_TRACE(Logger::WARNING, "Error decoding " <<
                        m_sFilename);
                // TODO: simulate eof.
            }
            m_pPacketData += Len1;
            m_PacketLenLeft -= Len1;
        }
/*
        cerr << "coded_picture_number: " << Frame.coded_picture_number <<
                ", display_picture_number: " << Frame.display_picture_number <<
                ", pts: " << Frame.pts << endl;

        cerr << "key_frame: " << Frame.key_frame << 
               ", pict_type: " << Frame.pict_type << endl;
        AVFrac spts = m_pVStream->pts;
        cerr << "Stream.pts: " << spts.val + double(spts.num)/spts.den << endl;
*/    
    }
}

bool FFMpegDecoder::getNextVideoPacket(AVPacket & Packet) {
    AVPacket CurPacket;
    int err = av_read_frame(m_pFormatContext, &CurPacket);
    if (err < 0) {
        return true;
    }
    while (CurPacket.stream_index != m_VStreamIndex) {
        av_free_packet(&CurPacket);
        int err = av_read_frame(m_pFormatContext, &CurPacket);
        if (err < 0) {
            return true;
        }
    }   
    Packet = CurPacket;
/*    if (Packet.pts != AV_NOPTS_VALUE) {
        cerr << "Packet.pts: " << 
            double(Packet.pts)*m_pFormatContext->pts_num/m_pFormatContext->pts_den << endl;
    }
*/
    return false;
}

}

