//
// $Id$
// 

#ifndef _AVGDFBDisplayEngine_H_
#define _AVGDFBDisplayEngine_H_

#include "AVGDFBFontManager.h"
#include "IAVGDisplayEngine.h"

#include <directfb/directfb.h>
#include <paintlib/plrect.h>

#include <string>

class PLDirectFBBmp;

class AVGDFBDisplayEngine: public IAVGDisplayEngine
{
    public:
        AVGDFBDisplayEngine();
        virtual ~AVGDFBDisplayEngine();
        virtual void init(int width, int height, bool isFullscreen, int bpp);
        virtual void teardown();

        virtual void render(AVGNode * pRootNode, 
                AVGFramerateManager * pFramerateManager, bool bRenderEverything);
        
        virtual void setClipRect();
        virtual bool setClipRect(const PLRect& rc);
        virtual const PLRect& getClipRect();
        virtual void blt32(PLBmp * pBmp, const PLRect* pSrcRect, 
                const PLPoint& pos, double opacity);
        virtual void blta8(PLBmp * pBmp, const PLRect* pSrcRect,
                const PLPoint& pos, double opacity, const PLPixel32& color);
        virtual void swapBuffers(const AVGRegion & UpdateRegion);

	    virtual PLBmp * createSurface();

        virtual int getWidth();
        virtual int getHeight();
        virtual int getBPP();

        virtual AVGFontManager * getFontManager();

        IDirectFB * getDFB();
        IDirectFBSurface * getPrimary();
        IDirectFBEventBuffer * getEventBuffer();
        void DFBErrorCheck(int avgcode, std::string where, DFBResult dfbcode); 

    private:
        void initDFB(int width, int height, bool isFullscreen, int bpp);
        void initLayer(int width, int height);
        void initInput();
        void initBackbuffer();
        void clear();
        void setDirtyRect(const PLRect& rc);

        void blt32(IDirectFBSurface * pSrc, const PLRect* pSrcRect, 
                const PLPoint& pos, double opacity, bool bAlpha);
        void blt(IDirectFBSurface * pSrc, const PLRect* pSrcRect, 
                const PLPoint& pos);

        void dumpSurface(IDirectFBSurface * pSurf, const std::string & name);
        
        int m_Width;
        int m_Height;
        bool m_IsFullscreen;
        int m_bpp;
        PLRect m_ClipRect;
        PLRect m_DirtyRect;

        IDirectFB * m_pDirectFB;
        IDirectFBWindow * m_pDFBWindow;
        IDirectFBDisplayLayer * m_pDFBLayer;
        IDirectFBSurface * m_pBackBuffer;
        
        IDirectFBEventBuffer * m_pEventBuffer;

        AVGDFBFontManager *m_pFontManager;
};

#endif //_AVGDFBDisplayEngine_H_
