//
// $Id$
// 

#ifndef _AVGVideo_H_
#define _AVGVideo_H_

#include "AVGNode.h"
#include "IAVGVideo.h"
#include "AVGVideoBase.h"
#include "AVGRect.h"

#include <paintlib/plbitmap.h>

#include <string>

class IAVGVideoDecoder;

//8d8abfe4-a725-4908-96a6-53c575f1f574
#define AVGVIDEO_CID \
{ 0x8d8abfe4, 0xa725, 0x4908, { 0x96, 0xa6, 0x53, 0xc5, 0x75, 0xf1, 0xf5, 0x74 } }

#define AVGVIDEO_CONTRACTID "@c-base.org/avgvideo;1"

class AVGVideo : public AVGVideoBase, public IAVGVideo
{
    public:
        NS_DECL_ISUPPORTS
        NS_DECL_IAVGVIDEO

        static AVGVideo * create();

        AVGVideo ();
        virtual ~AVGVideo ();
        
        NS_IMETHOD GetType(PRInt32 *_retval);

        virtual void init (const std::string& id, const std::string& filename, 
           bool bLoop, bool bOverlay, 
           IAVGDisplayEngine * pEngine, AVGContainer * pParent, AVGPlayer * pPlayer);
        virtual std::string getTypeStr ();

    private:
        void initVideoSupport();

        bool renderToBmp(PLBmp * pBmp);
        virtual void renderToBackbuffer(PLBmp & pBackBufferBmp, 
                const AVGDRect& vpt);
        bool canRenderToBackbuffer(int BitsPerPixel);
        void seek(int DestFrame);
        void advancePlayback();
       
        virtual bool open(int* pWidth, int* pHeight);
        virtual void close();
        virtual double getFPS();

        std::string m_Filename;
        bool m_bLoop;

        int m_CurFrame;
        bool m_bEOF;

        IAVGVideoDecoder * m_pDecoder;

        static bool m_bInitialized;
};

#endif 

