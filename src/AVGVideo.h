//
// $Id$
// 

#ifndef _AVGVideo_H_
#define _AVGVideo_H_

#include "AVGNode.h"
#include "IAVGVideo.h"

#include <libmpeg3.h>

#include <string>

class PLBmp;
//8d8abfe4-a725-4908-96a6-53c575f1f574
#define AVGVIDEO_CID \
{ 0x8d8abfe4, 0xa725, 0x4908, { 0x96, 0xa6, 0x53, 0xc5, 0x75, 0xf1, 0xf5, 0x74 } }

#define AVGVIDEO_CONTRACTID "@c-base.org/avgvideo;1"

class AVGVideo : public AVGNode, IAVGVideo
{
	public:
        NS_DECL_ISUPPORTS
        NS_DECL_IAVGVIDEO

        static AVGVideo * create();

        AVGVideo ();
        virtual ~AVGVideo ();
        
        NS_IMETHOD GetType(PRInt32 *_retval);

        virtual void init (const std::string& id, int x, int y, int z, 
           int width, int height, double opacity, const std::string& filename, 
           bool bLoop, bool bOverlay, 
           AVGDFBDisplayEngine * pEngine, AVGContainer * pParent);
        virtual void prepareRender (int time, const PLRect& parent);
        virtual void render (const PLRect& Rect);
        bool obscures (const PLRect& Rect, int z);
        virtual std::string getTypeStr ();
        virtual std::string dump (int indent = 0);

    protected:        
        virtual PLPoint getPreferredMediaSize();
    
    private:
        void open (int* pWidth, int* pHeight);
        typedef enum VideoState {Unloaded, Paused, Playing};
        void changeState(VideoState NewState);
        void readFrame();
        void renderToBackbuffer();
        void initOverlay();
        void renderToOverlay();
        void advancePlayback();
        
        std::string m_Filename;
        int m_Width;
        int m_Height;
        bool m_bLoop;
        bool m_bOverlay;

        PLBmp * m_pBmp;
        mpeg3_t* m_pMPEG;
        bool m_bFrameAvailable;

        VideoState m_State;
        int m_CurFrame;
        PLPoint m_PreferredSize;
};

#endif 

