//
// $Id$
// 

#ifndef _IAVGVideoDecoder_H_
#define _IAVGVideoDecoder_H_

#include "AVGRect.h"

#include <paintlib/plbitmap.h>

#include <string>

class PLBmp;

class IAVGVideoDecoder
{
    public:
        virtual bool open(const std::string& sFilename, 
                int* pWidth, int* pHeight) = 0;
        virtual void close() = 0;
        virtual void seek(int DestFrame, int CurFrame) = 0;
        virtual int getNumFrames() = 0;
        virtual double getFPS() = 0;

        virtual bool renderToBmp(PLBmp * pBmp, const AVGDRect* pVpt=0) = 0;
        virtual bool canRenderToBuffer(int BPP) = 0;
};

#endif 

