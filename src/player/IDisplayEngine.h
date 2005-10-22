//
// $Id$
//

#ifndef _IDisplayEngine_H_
#define _IDisplayEngine_H_

#include "../graphics/Rect.h"
#include "../graphics/Pixel32.h"
#include "../graphics/Bitmap.h"

#include <string>

namespace avg {

class ISurface;

class Region;
class FramerateManager;
class Node;
class AVGNode;

class IDisplayEngine
{	
    public:
        typedef enum BlendMode {BLEND_BLEND, BLEND_ADD, BLEND_MIN, BLEND_MAX};

        virtual ~IDisplayEngine(){};
        virtual void init(int width, int height, bool isFullscreen, 
                int bpp, int WindowWidth, int WindowHeight) = 0;
        virtual void teardown() = 0;

        virtual void render(AVGNode * pRootNode, 
                FramerateManager * pFramerateManager, 
                bool bRenderEverything) = 0;
        
        virtual void setClipRect() = 0;
        virtual bool pushClipRect(const DRect& rc, bool bClip) = 0;
        virtual void popClipRect() = 0;
        virtual const DRect& getClipRect() = 0;
        virtual void blt32(ISurface * pSurface, const DRect* pDestRect, 
                double opacity, double angle, const DPoint& pivot,
                BlendMode Mode) = 0;
        virtual void blta8(ISurface * pSurface, const DRect* pDestRect, 
                double opacity, const Pixel32& color, double angle,
                const DPoint& pivot, BlendMode Mode) = 0;
        virtual ISurface * createSurface() = 0;
        virtual void surfaceChanged(ISurface * pSurface) {};

        virtual int getWidth() = 0;
        virtual int getHeight() = 0;
        virtual int getBPP() = 0;

        // True if the bpp given are supported.
        virtual bool supportsBpp(int bpp) = 0; 
        // True if pixel order is RGB; BGR otherwise.
        virtual bool hasRGBOrdering() = 0; 
        virtual bool isYCbCrSupported() = 0; 
        virtual void showCursor (bool bShow) = 0;

        virtual BitmapPtr screenshot () = 0;

};

}

#endif //_IDisplayEngine_H_

