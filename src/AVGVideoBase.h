//
// $Id$
// 

#ifndef _AVGVideoBase_H_
#define _AVGVideoBase_H_

#include "AVGNode.h"
#include "IAVGVideoBase.h"
#include "AVGRect.h"

#include <paintlib/plbitmap.h>

#include <string>

class PLBmp;
class IAVGSurface;

class AVGVideoBase : public IAVGVideoBase, public AVGNode
{
    public:
        NS_DECL_ISUPPORTS
        NS_DECL_IAVGVIDEOBASE

        AVGVideoBase ();
        virtual ~AVGVideoBase ();
        
        virtual void init (const std::string& id, bool bOverlay, 
           IAVGDisplayEngine * pEngine, AVGContainer * pParent, AVGPlayer * pPlayer);
        virtual void prepareRender (int time, const AVGDRect& parent);
        virtual void render (const AVGDRect& Rect);
        bool obscures (const AVGDRect& Rect, int z);
        virtual std::string dump (int indent = 0);

    protected:        
        virtual AVGDPoint getPreferredMediaSize();
        typedef enum VideoState {Unloaded, Paused, Playing};
        virtual VideoState getState();
        void setFrameAvailable(bool bAvailable);
        void changeState(VideoState NewState);
        int getMediaWidth();
        int getMediaHeight();
   
    private:
        void renderToBackbuffer();
        void open();

        virtual bool renderToSurface(IAVGSurface * pSurface) = 0;
        virtual bool canRenderToBackbuffer(int BitsPerPixel) = 0;
        virtual void open(int* pWidth, int* pHeight) = 0;
        virtual void close() = 0;
        virtual double getFPS() = 0;
       
        int m_Width;
        int m_Height;

        bool m_bFrameAvailable;

        VideoState m_State;
};

#endif 

