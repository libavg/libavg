//
// $Id$
// 

#ifndef _AVGDFBDisplayEngine_H_
#define _AVGDFBDisplayEngine_H_

#include "AVGDFBFontManager.h"
#include "IAVGEventSource.h"
#include "IAVGDisplayEngine.h"

#include <directfb/directfb.h>
#include <paintlib/plrect.h>

#include <string>

class PLDirectFBBmp;

class AVGDFBDisplayEngine: public IAVGDisplayEngine, public IAVGEventSource
{
    public:
        AVGDFBDisplayEngine();
        virtual ~AVGDFBDisplayEngine();

        // From IAVGDisplayEngine
        virtual void init(int width, int height, bool isFullscreen, int bpp);
        virtual void teardown();

        virtual void render(AVGNode * pRootNode, 
                AVGFramerateManager * pFramerateManager, bool bRenderEverything);
        
        virtual void setClipRect();
        virtual bool setClipRect(const PLRect& rc);
        virtual const PLRect& getClipRect();
        virtual void blt32(PLBmp * pBmp, const PLRect* pDestRect, double opacity);
        virtual void blta8(PLBmp * pBmp, const PLRect* pDestRect, 
                double opacity, const PLPixel32& color);

	virtual PLBmp * createSurface();

        virtual int getWidth();
        virtual int getHeight();
        virtual int getBPP();

        virtual AVGFontManager * getFontManager();

        // From IAVGEventSource
        virtual std::vector<AVGEvent *> pollEvents();
       
        // Methods specific to AVGDFBEventSource
        IDirectFB * getDFB();
        IDirectFBSurface * getPrimary();
        void DFBErrorCheck(int avgcode, std::string where, DFBResult dfbcode); 

    private:
        void initDFB(int width, int height, bool isFullscreen, int bpp);
        void initLayer(int width, int height);
        void initInput();
        void initBackbuffer();
        void clear();
        void setDirtyRect(const PLRect& rc);
        virtual void swapBuffers(const AVGRegion & UpdateRegion);

        void blt32(IDirectFBSurface * pSrc, const PLRect* pDestRect, 
                double opacity, bool bAlpha);
        void blt(IDirectFBSurface * pSrc, const PLRect* pDestRect);

        void dumpSurface(IDirectFBSurface * pSurf, const std::string & name);
        
        AVGEvent * createEvent(const char * pTypeName);
        int translateModifiers(DFBInputDeviceModifierMask DFBModifiers);
        AVGEvent * createEvent(DFBWindowEvent* pdfbwEvent);
        
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
