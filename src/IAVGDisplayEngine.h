//
// $Id$
//

#ifndef _IAVGDisplayEngine_H_
#define _IAVGDisplayEngine_H_

class PLBmp;
class PLPixel32;

class AVGRegion;
class AVGFramerateManager;
class AVGNode;
class AVGFontManager;

#include "AVGRect.h"

class IAVGDisplayEngine
{	
    public:
        virtual ~IAVGDisplayEngine(){};
        virtual void init(int width, int height, bool isFullscreen, int bpp) = 0;
        virtual void teardown() = 0;

        virtual void render(AVGNode * pRootNode, 
                AVGFramerateManager * pFramerateManager, 
                bool bRenderEverything) = 0;
        
        virtual void setClipRect() = 0;
        virtual bool pushClipRect(const AVGDRect& rc, bool bClip) = 0;
        virtual void popClipRect() = 0;
        virtual const AVGDRect& getClipRect() = 0;
        virtual void blt32(PLBmp * pBmp, const AVGDRect* pDestRect, 
                double opacity, double angle, const AVGDPoint& pivot) = 0;
        virtual void blta8(PLBmp * pBmp, const AVGDRect* pDestRect, 
                double opacity, const PLPixel32& color, double angle,
                const AVGDPoint& pivot) = 0;
        virtual void bltYUV(PLBmp * pBmp, const AVGDRect* pDestRect, 
                double opacity, double angle, const AVGDPoint& pivot) {};
        virtual PLBmp * createSurface() = 0;
        virtual void surfaceChanged(PLBmp* pBmp) {};

        virtual int getWidth() = 0;
        virtual int getHeight() = 0;
        virtual int getBPP() = 0;

        virtual AVGFontManager * getFontManager() = 0;
        virtual bool hasYUVSupport() = 0;
};

#endif //_IAVGDisplayEngine_H_

