//
// $Id$
// 

#include "AVGFFMpegDecoder.h"

#include "IAVGDisplayEngine.h"
#ifdef AVG_ENABLE_DFB
#include "AVGDFBDisplayEngine.h"
#endif
#include "AVGException.h"
#include "AVGPlayer.h"
#include "AVGLogger.h"

#include <paintlib/plbitmap.h>
#include <paintlib/pldirectfbbmp.h>
//#include <paintlib/planybmp.h>
//#include <paintlib/Filter/plfilterfill.h>

#include <iostream>
#include <sstream>
#include <unistd.h>

using namespace std;

bool AVGFFMpegDecoder::m_bInitialized = false;

AVGFFMpegDecoder::AVGFFMpegDecoder ()
    : m_pFormatContext(0),
      m_pPacketData(0),
      m_pVStream(0)
{
    initVideoSupport();
}

AVGFFMpegDecoder::~AVGFFMpegDecoder ()
{
    if (m_pFormatContext) {
        close();
    }
}


void avcodecError(const string & filename, int err)
{
    switch(err) {
        case AVERROR_NUMEXPECTED:
            AVG_TRACE(AVGPlayer::DEBUG_ERROR, 
                    filename << ": Incorrect image filename syntax.");
            AVG_TRACE(AVGPlayer::DEBUG_ERROR, 
                    "Use '%%d' to specify the image number:");
            AVG_TRACE(AVGPlayer::DEBUG_ERROR, 
                    "  for img1.jpg, img2.jpg, ..., use 'img%%d.jpg';");
            AVG_TRACE(AVGPlayer::DEBUG_ERROR, 
                    "  for img001.jpg, img002.jpg, ..., use 'img%%03d.jpg'."); 
            break;
        case AVERROR_INVALIDDATA:
            AVG_TRACE(AVGPlayer::DEBUG_ERROR, 
                    filename << ": Error while parsing header");
            break;
        case AVERROR_NOFMT:
            AVG_TRACE(AVGPlayer::DEBUG_ERROR, 
                    filename << ": Unknown format");
            break;
        default:
            AVG_TRACE(AVGPlayer::DEBUG_ERROR, 
                    filename << ": Error while opening file");
            break;
    }
    // TODO: Continue without video.
    exit(-1);
}

/*
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
*/

bool AVGFFMpegDecoder::open (const std::string& sFilename, 
        int* pWidth, int* pHeight)
{
    AVFormatParameters params;
    int err;
    m_sFilename = sFilename;

    AVG_TRACE(AVGPlayer::DEBUG_PROFILE, "Opening " << sFilename);
    memset(&params, 0, sizeof(params));
    params.image_format = 0;

    err = av_open_input_file(&m_pFormatContext, sFilename.c_str(), 
            0, 0, &params);
    if (err < 0) {
        avcodecError(sFilename, err);
    }
    err = av_find_stream_info(m_pFormatContext);
    if (err < 0) {
        AVG_TRACE(AVGPlayer::DEBUG_ERROR, 
                    sFilename << ": Could not find codec parameters.");
        return false;
    }

    m_VStreamIndex = -1;
    for(int i = 0; i < m_pFormatContext->nb_streams; i++) {
        AVCodecContext *enc = &m_pFormatContext->streams[i]->codec;
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
//    dump_format(m_pFormatContext, 0, m_Filename.c_str(), 0);
//    dump_stream_info(pFormatContext);
    if (m_VStreamIndex < 0) {
        AVG_TRACE(AVGPlayer::DEBUG_ERROR, 
                    sFilename << " does not contain any video streams.");
        return false;
    }                
    AVCodecContext *enc;
    enc = &(m_pFormatContext->streams[m_VStreamIndex]->codec);
//    enc->debug = 0x0001; // see avcodec.h
    AVCodec * codec = avcodec_find_decoder(enc->codec_id);
    if (!codec ||
        avcodec_open(enc, codec) < 0)
    {
        AVG_TRACE(AVGPlayer::DEBUG_ERROR, 
                    sFilename << ": could not open codec (?!).");
        return false;
    }                
    m_pVStream = m_pFormatContext->streams[m_VStreamIndex];
    
    *pWidth =  m_pVStream->codec.width;
    *pHeight = m_pVStream->codec.height;
    
    m_bFirstPacket = true;
    m_PacketLenLeft = 0;
    m_bEOF = false;
    return true;
} 

void AVGFFMpegDecoder::close() 
{
    AVCodecContext * enc;
    enc = &(m_pFormatContext->streams[m_VStreamIndex]->codec);
    if (!m_bFirstPacket) {
        av_free_packet(&m_Packet);
    }
    m_pPacketData = 0;
    avcodec_close(enc);
    m_pVStream = 0;
    av_close_input_file(m_pFormatContext);
    m_pFormatContext = 0;
}

void AVGFFMpegDecoder::seek(int DestFrame, int CurFrame) 
{
    // This is a stupid seek that just reads frames till it arrives at the 
    // correct one. ffmpeg doesn't seem to give us any other choice.
    if (DestFrame < CurFrame) {
        close();
        int Width, Height; // Not needed, we're just seeking. 
        bool bOK = open(m_sFilename, &Width, &Height);
        if (!bOK) {
            AVG_TRACE(AVGPlayer::DEBUG_ERROR, 
                    m_sFilename << ": Open during seek failed. Aborting.");
            exit(-1);
        }
        CurFrame = 0;
    }
    m_pVStream->codec.hurry_up = 0;
//    int64_t DestTime = ((DestFrame*1000)/m_pVStream->r_frame_rate);
//    cerr << "DestTime: " << DestTime << 
//            ", FrameRate: " << m_pVStream->r_frame_rate << endl;
    AVFrame Frame;
    for (int i = CurFrame; i<DestFrame; i++) {
//cerr << "coded_picture_number: " << Frame.coded_picture_number << endl;
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
                    AVG_TRACE(AVGPlayer::DEBUG_WARNING, "Decoding error." <<
                            m_sFilename);
                    // TODO: simulate eof.
                }
                m_pPacketData += Len1;
                m_PacketLenLeft -= Len1;
            }
        }

        if (m_bEOF) {
            AVG_TRACE(AVGPlayer::DEBUG_ERROR, 
                    m_sFilename << ": Seek beyond end of file (frame " << 
                    DestFrame << ").");
            // TODO: changeState(Unloaded);
        }
    }
    m_pVStream->codec.hurry_up = 0;
}

int AVGFFMpegDecoder::getNumFrames()
{
    // TODO: This doesn't always seem to work...
    return m_pVStream->r_frame_rate*(m_pVStream->duration/AV_TIME_BASE);
}

double AVGFFMpegDecoder::getFPS()
{
    return m_pVStream->r_frame_rate;
}

bool AVGFFMpegDecoder::renderToBmp(PLBmp * pBmp, bool bHasRGBOrdering,
        const AVGDRect* pVpt)
{
/* Speedup possibilities:
    fast YUV->RGB conversion? incl. scaling?
    draw_horiz_band?
*/
    AVGDRect Vpt (0, 0, pBmp->GetWidth(), pBmp->GetHeight());
    if (pVpt != 0) {
        Vpt = *pVpt;
    }
    AVFrame Frame;
    readFrame(Frame);
    if (!m_bEOF) {
        AVPicture DestPict;
        int x1 = int(Vpt.tl.x);
        int y1 = int(Vpt.tl.y);
        PLBYTE ** ppDestLines = pBmp->GetLineArray();
        PLBYTE * pDestBits = ppDestLines[y1]+3*x1;
        DestPict.data[0] = pDestBits;
        DestPict.data[1] = pDestBits+1;
        DestPict.data[2] = pDestBits+2;
        DestPict.linesize[0] = ppDestLines[1] - ppDestLines[0];
        DestPict.linesize[1] = DestPict.linesize[0];   
        DestPict.linesize[2] = DestPict.linesize[0];  
        int DestFmt;
        if (bHasRGBOrdering) {
            DestFmt = PIX_FMT_RGB24;
        } else {
            DestFmt = PIX_FMT_BGR24;
        }
        img_convert(&DestPict, DestFmt,
                (AVPicture*)&Frame, m_pVStream->codec.pix_fmt,
                m_pVStream->codec.width, m_pVStream->codec.height);
    }
    return m_bEOF;
}

bool AVGFFMpegDecoder::canRenderToBuffer(int BPP)
{
#ifdef AVG_ENABLE_DFB
    return (BPP == 24);
#else
    return false;
#endif
}

void AVGFFMpegDecoder::initVideoSupport()
{
    if (!m_bInitialized) {
        av_register_all();
        m_bInitialized = true;
    }
}

void AVGFFMpegDecoder::readFrame(AVFrame& Frame)
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
                AVG_TRACE(AVGPlayer::DEBUG_WARNING, "Error decoding " <<
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

bool AVGFFMpegDecoder::getNextVideoPacket(AVPacket & Packet) {
    AVPacket CurPacket;
    int err = av_read_packet(m_pFormatContext, &CurPacket);
    if (err < 0) {
        return true;
    }
    while (CurPacket.stream_index != m_VStreamIndex) {
        av_free_packet(&CurPacket);
        int err = av_read_packet(m_pFormatContext, &CurPacket);
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


