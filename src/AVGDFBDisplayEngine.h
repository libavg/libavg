//
// $Id$
// 

#ifndef _AVGDFBDisplayEngine_H_
#define _AVGDFBDisplayEngine_H_

#include <directfb/directfb.h>

class PLBmp;
class PLDirectFBBmp;
class PLPoint;
class PLRect;

class AVGDFBDisplayEngine
{
    public:
        AVGDFBDisplayEngine();
        virtual ~AVGDFBDisplayEngine();
        virtual void init(int width, int height, bool isFullscreen);
        virtual void teardown();

        virtual void setClipRect(const PLRect& rc);
        virtual void render(PLBmp * pBmp, const PLPoint& pos, double opacity);
        virtual void swapBuffers();

	    virtual PLBmp * createSurface();

        IDirectFB * getDFB();
        IDirectFBEventBuffer * getEventBuffer();
        void DFBErrorCheck(int avgcode, DFBResult dfbcode); 

    private:
        void initInput();

        int m_Width;
        int m_Height;
        bool m_IsFullscreen;

        IDirectFB * m_pDirectFB;
        IDirectFBWindow * m_pDFBWindow;
        IDirectFBDisplayLayer * m_pDFBLayer;
        IDirectFBSurface * m_pPrimary;

        IDirectFBEventBuffer * m_pEventBuffer;
};

#endif //_AVGDFBDisplayEngine_H_
