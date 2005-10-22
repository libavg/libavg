//
// $Id$
// 

#ifndef _Video_H_
#define _Video_H_

#include "Node.h"
#include "VideoBase.h"
#include "../graphics/Rect.h"

#include <string>

namespace avg {

class IVideoDecoder;

class Video : public VideoBase
{
    public:
        Video ();
        Video (const xmlNodePtr xmlNode, Container * pParent);
        virtual ~Video ();
        
        virtual void init (IDisplayEngine * pEngine, Container * pParent, 
                Player * pPlayer);

        const std::string& getHRef() const
        {
            return m_Filename;
        }
        
        int getNumFrames() const;
        int getCurFrame() const;
        void seekToFrame(int num);
        bool getLoop() const;
        virtual bool isYCbCrSupported();

        virtual std::string getTypeStr ();

    private:
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

