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

    private:
        void errorCheck(int avgcode, DFBResult dfbcode); 
        int m_Width;
        int m_Height;
        bool m_IsFullscreen;

        IDirectFB * m_pDirectFB;
        IDirectFBSurface * m_pPrimary;
};

#endif //_AVGDFBDisplayEngine_H_
