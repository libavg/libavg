//
// $Id$
// 

#ifndef _AVGFFMpegDecoder_H_
#define _AVGFFMpegDecoder_H_

#include "IAVGVideoDecoder.h"

#include <ffmpeg/avformat.h>

class AVGFFMpegDecoder: public IAVGVideoDecoder
{
    public:
        AVGFFMpegDecoder();
        virtual ~AVGFFMpegDecoder();
        virtual bool open(const std::string& sFilename, 
                int* pWidth, int* pHeight);
        virtual void close();
        virtual void seek(int DestFrame, int CurFrame);
        virtual int getNumFrames();
        virtual double getFPS();
        virtual bool renderToBmp(PLBmp * pBmp);
        virtual bool renderToBuffer(PLBYTE * pSurfBits, int Pitch, 
                int BytesPerPixel, const AVGDRect& vpt);
        virtual bool canRenderToBuffer(int BPP);

    private:
        void initVideoSupport();
        void readFrame(AVFrame& Frame);
        bool getNextVideoPacket(AVPacket & Packet);

        AVFormatContext * m_pFormatContext;
        int m_VStreamIndex;
        AVStream * m_pVStream;
        bool m_bEOF;

        unsigned char * m_pPacketData;
        AVPacket m_Packet;
        int m_PacketLenLeft;
        bool m_bFirstPacket;
        std::string m_sFilename;

        static bool m_bInitialized;
};

#endif 

