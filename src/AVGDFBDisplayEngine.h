//
// $Id$
// 

#ifndef _AVGDFBDisplayEngine_H_
#define _AVGDFBDisplayEngine_H_

#include "AVGFontManager.h"

#include <directfb/directfb.h>
#include <paintlib/plrect.h>

#include <string>

class PLBmp;
class PLDirectFBBmp;
class PLPoint;

class AVGRegion;

class AVGDFBDisplayEngine
{
    public:
        AVGDFBDisplayEngine();
        virtual ~AVGDFBDisplayEngine();
        virtual void init(int width, int height, bool isFullscreen, bool bDebugBlts);
        virtual void teardown();

        virtual void setClipRect();
        virtual bool setClipRect(const PLRect& rc);
        virtual const PLRect& getClipRect();
        virtual void setDirtyRect(const PLRect& rc);
        virtual void clear();
        virtual void render(PLBmp * pBmp, const PLPoint& pos, double opacity);
        virtual void render(IDirectFBSurface * pSrc, const PLPoint& pos, 
                double opacity, bool bAlpha);
        virtual void swapBuffers(const AVGRegion & UpdateRegion);

	    virtual PLBmp * createSurface();
        void dumpSurface(IDirectFBSurface * pSurf, const std::string & name);

        IDirectFB * getDFB();
        IDirectFBSurface * getPrimary();
        IDirectFBEventBuffer * getEventBuffer();
        AVGFontManager * getFontManager();
        int getWidth();
        int getHeight();
        void DFBErrorCheck(int avgcode, std::string where, DFBResult dfbcode); 

    private:
        void initDFB(int width, int height, bool isFullscreen);
        void initLayer(int width, int height);
        void initInput();
        void initBackbuffer();

        int m_Width;
        int m_Height;
        bool m_IsFullscreen;
        PLRect m_ClipRect;
        PLRect m_DirtyRect;
        bool m_bDebugBlts;

        IDirectFB * m_pDirectFB;
        IDirectFBWindow * m_pDFBWindow;
        IDirectFBDisplayLayer * m_pDFBLayer;
        IDirectFBSurface * m_pBackBuffer;
        
        IDirectFBEventBuffer * m_pEventBuffer;

        AVGFontManager *m_pFontManager;
};

#endif //_AVGDFBDisplayEngine_H_
