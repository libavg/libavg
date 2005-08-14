//
// $Id$
// 

#ifndef _FFMpegDecoder_H_
#define _FFMpegDecoder_H_

#include "IVideoDecoder.h"

#include <ffmpeg/avformat.h>

namespace avg {

class FFMpegDecoder: public IVideoDecoder
{
    public:
        FFMpegDecoder();
        virtual ~FFMpegDecoder();
        virtual bool open(const std::string& sFilename, 
                int* pWidth, int* pHeight);
        virtual void close();
        virtual void seek(int DestFrame, int CurFrame);
        virtual int getNumFrames();
        virtual double getFPS();
        virtual bool renderToBmp(BitmapPtr pBmp);
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

}
#endif 

