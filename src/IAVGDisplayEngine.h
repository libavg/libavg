//
// $Id$
//

#ifndef _IAVGDisplayEngine_H_
#define _IAVGDisplayEngine_H_

class IAVGSurface;
class PLPixel32;

class AVGRegion;
class AVGFramerateManager;
class AVGNode;
class AVGFontManager;

#include "AVGRect.h"

#include <paintlib/plbitmap.h>

#include <string>

class IAVGDisplayEngine
{	
    public:
        virtual ~IAVGDisplayEngine(){};
        virtual void init(int width, int height, bool isFullscreen, int bpp,
                const std::string & sFontPath) = 0;
        virtual void teardown() = 0;

        virtual void render(AVGNode * pRootNode, 
                AVGFramerateManager * pFramerateManager, 
                bool bRenderEverything) = 0;
        
        virtual void setClipRect() = 0;
        virtual bool pushClipRect(const AVGDRect& rc, bool bClip) = 0;
        virtual void popClipRect() = 0;
        virtual const AVGDRect& getClipRect() = 0;
        virtual void blt32(IAVGSurface * pSurface, const AVGDRect* pDestRect, 
                double opacity, double angle, const AVGDPoint& pivot) = 0;
        virtual void blta8(IAVGSurface * pSurface, const AVGDRect* pDestRect, 
                double opacity, const PLPixel32& color, double angle,
                const AVGDPoint& pivot) = 0;
        virtual void bltYUV(IAVGSurface * pSurface, const AVGDRect* pDestRect, 
                double opacity, double angle, const AVGDPoint& pivot) {};
        virtual IAVGSurface * createSurface() = 0;
        virtual void surfaceChanged(IAVGSurface * pSurface) {};

        virtual int getWidth() = 0;
        virtual int getHeight() = 0;
        virtual int getBPP() = 0;

        virtual AVGFontManager * getFontManager() = 0;
        virtual bool hasYUVSupport() = 0;
        // True if the bpp given are supported.
        virtual bool supportsBpp(int bpp) = 0; 
        // True if pixel order is RGB; BGR otherwise.
        virtual bool hasRGBOrdering() = 0; 
        virtual void showCursor (bool bShow) = 0;

        virtual void screenshot (const std::string& sFilename,
                PLBmp& Bmp) = 0;
};

#endif //_IAVGDisplayEngine_H_

