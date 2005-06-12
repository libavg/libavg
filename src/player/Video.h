//
// $Id$
// 

#ifndef _Video_H_
#define _Video_H_

#include "Node.h"
#include "VideoBase.h"
#include "Rect.h"

#include <paintlib/plbitmap.h>

#include <string>

namespace avg {

class IVideoDecoder;

class Video : public VideoBase
{
    public:
        Video ();
        virtual ~Video ();
        
        virtual void init (IDisplayEngine * pEngine, Container * pParent, 
                Player * pPlayer);

        int getNumFrames();
        int getCurFrame();
        void seekToFrame(int num);

        virtual std::string getTypeStr ();
        virtual JSFactoryBase* getFactory();

    private:
        friend class VideoFactory;
        void initVideoSupport();

        bool renderToSurface(ISurface * pSurface);
        bool canRenderToBackbuffer(int BitsPerPixel);
        void seek(int DestFrame);
        void advancePlayback();
       
        virtual void open(int* pWidth, int* pHeight);
        virtual void close();
        virtual double getFPS();

        std::string m_Filename;
        bool m_bLoop;

        int m_CurFrame;
        bool m_bEOF;

        IVideoDecoder * m_pDecoder;

        static bool m_bInitialized;
};

}

#endif 

