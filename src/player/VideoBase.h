//
// $Id$
// 

#ifndef _VideoBase_H_
#define _VideoBase_H_

#include "RasterNode.h"
#include "../graphics/Rect.h"

#include <string>

class ISurface;

namespace avg {

class VideoBase : public RasterNode
{
    public:
        virtual ~VideoBase ();
        
        virtual void init (IDisplayEngine * pEngine, Container * pParent, 
                Player * pPlayer);

        void play();
        void stop();
        void pause();
        virtual double getFPS() = 0;
        virtual bool isYCbCrSupported() = 0;
        
        virtual void prepareRender (int time, const DRect& parent);
        virtual void render (const DRect& Rect);
        bool obscures (const DRect& Rect, int z);
        virtual std::string dump (int indent = 0);
        
    protected:        
        VideoBase ();
        VideoBase (const xmlNodePtr xmlNode, Container * pParent);
        virtual DPoint getPreferredMediaSize();
        typedef enum VideoState {Unloaded, Paused, Playing};
        virtual VideoState getState() const;
        void setFrameAvailable(bool bAvailable);
        void changeState(VideoState NewState);
        int getMediaWidth();
        int getMediaHeight();
   
    private:
        void renderToBackbuffer();
        void open();

        virtual bool renderToSurface(ISurface * pSurface) = 0;
        virtual bool canRenderToBackbuffer(int BitsPerPixel) = 0;
        virtual void open(int* pWidth, int* pHeight) = 0;
        virtual void close() = 0;

        VideoState m_State;
       
        int m_Width;
        int m_Height;

        bool m_bFrameAvailable;
};

}

#endif 

