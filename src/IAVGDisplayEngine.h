//
// $Id$
//

#ifndef _IAVGDisplayEngine_H_
#define _IAVGDisplayEngine_H_

class PLBmp;
class PLRect;
class PLPixel32;
class PLPoint;

class AVGRegion;
class AVGFramerateManager;
class AVGNode;
class AVGFontManager;

class IAVGDisplayEngine
{	
    public:
        virtual ~IAVGDisplayEngine(){};
        virtual void init(int width, int height, bool isFullscreen, int bpp) = 0;
        virtual void teardown() = 0;

        virtual void render(AVGNode * pRootNode, 
                AVGFramerateManager * pFramerateManager, bool bRenderEverything) = 0;
        
        virtual void setNodeRect() = 0;
        virtual bool setNodeRect(const PLRect& rc, bool bClip) = 0;
        virtual const PLRect& getClipRect() = 0;
        virtual void blt32(PLBmp * pBmp, const PLRect* pDestRect, 
                double opacity, double angle, const PLPoint& pivot) = 0;
        virtual void blta8(PLBmp * pBmp, const PLRect* pDestRect, 
                double opacity, const PLPixel32& color, double angle,
                const PLPoint& pivot) = 0;
        virtual PLBmp * createSurface() = 0;
        virtual void surfaceChanged(PLBmp* pBmp) {};

        virtual int getWidth() = 0;
        virtual int getHeight() = 0;
        virtual int getBPP() = 0;

        virtual AVGFontManager * getFontManager() = 0;
};

#endif //_IAVGDisplayEngine_H_

