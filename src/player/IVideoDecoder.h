//
// $Id$
// 

#ifndef _IVideoDecoder_H_
#define _IVideoDecoder_H_

#include "../graphics/Bitmap.h"

#include <string>

namespace avg {

class IVideoDecoder
{
    public:
        virtual ~IVideoDecoder() {};
        virtual bool open(const std::string& sFilename, 
                int* pWidth, int* pHeight) = 0;
        virtual void close() = 0;
        virtual void seek(int DestFrame, int CurFrame) = 0;
        virtual int getNumFrames() = 0;
        virtual double getFPS() = 0;

        virtual bool renderToBmp(BitmapPtr pBmp) = 0;
        virtual bool canRenderToBuffer(int BPP) = 0;
        virtual bool isYCbCrSupported() = 0;
};

}
#endif 

