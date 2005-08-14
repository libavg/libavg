//
// $Id$
// 

#ifndef _DFBDisplayEngine_H_
#define _DFBDisplayEngine_H_

#include "IEventSource.h"
#include "IDisplayEngine.h"

#include <directfb/directfb.h>

#include <string>

namespace avg {

class DFBDisplayEngine: public IDisplayEngine, public IEventSource
{
    public:
        DFBDisplayEngine();
        virtual ~DFBDisplayEngine();

        // From IDisplayEngine
        virtual void init(int width, int height, bool isFullscreen, int bpp,
                int WindowWidth, int WindowHeight);
        virtual void teardown();

        virtual void render(AVGNode * pRootNode, 
                FramerateManager * pFramerateManager, bool bRenderEverything);
        
        virtual void setClipRect();
        virtual bool pushClipRect(const DRect& rc, bool bClip);
        virtual void popClipRect();
        virtual const DRect& getClipRect();
        virtual void blt32(ISurface * pSurface, const DRect* pDestRect, 
                double opacity, double angle, const DPoint& pivot,
                BlendMode Mode);
        virtual void blta8(ISurface * pSurface, const DRect* pDestRect, 
                double opacity, const Pixel32& color, double angle,
                const DPoint& pivot, BlendMode Mode);

        virtual ISurface * createSurface();

        IDirectFBSurface * getPrimary();
        void DFBErrorCheck(int avgcode, std::string where, DFBResult dfbcode); 

        virtual int getWidth();
        virtual int getHeight();
        virtual int getBPP();

        virtual bool supportsBpp(int bpp);
        virtual bool hasRGBOrdering();

        virtual void showCursor (bool bShow);
        virtual BitmapPtr screenshot ();

        // From IEventSource
        virtual std::vector<Event *> pollEvents();
       
    private:
        void initDFB(int width, int height, bool isFullscreen, int bpp);
        void initLayer(int width, int height);
        void initInput();
        void initBackbuffer();
        void clear();
        void setDirtyRect(const DRect& rc);
        virtual void swapBuffers(const Region & UpdateRegion);

        void blt32(IDirectFBSurface * pSrc, const DRect* pDestRect, 
                double opacity, bool bAlpha, BlendMode Mode);
        void blt(IDirectFBSurface * pSrc, const DRect* pDestRect);

        void dumpSurface(IDirectFBSurface * pSurf, const std::string & name);

        Event * createEvent(const char * pTypeName);
        int translateModifiers(DFBInputDeviceModifierMask DFBModifiers);
        Event * createEvent(DFBWindowEvent* pdfbwEvent);
      
        int m_Width;
        int m_Height;
        bool m_IsFullscreen;
        int m_bpp;
        DRect m_ClipRect;
        DRect m_DirtyRect;

        IDirectFB * m_pDirectFB;
        IDirectFBWindow * m_pDFBWindow;
        IDirectFBDisplayLayer * m_pDFBLayer;
        IDirectFBSurface * m_pBackBuffer;
        
        IDirectFBEventBuffer * m_pEventBuffer;
};

}

#endif //_DFBDisplayEngine_H_
