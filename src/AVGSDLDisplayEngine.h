//
// $Id$
// 

#ifndef _AVGSDLDisplayEngine_H_
#define _AVGSDLDisplayEngine_H_

#include <SDL/SDL.h>

class PLBmp;
class PLSDLBmp;
class PLPoint;
class PLRect;

class AVGSDLDisplayEngine
{
	public:
        AVGSDLDisplayEngine();
        virtual ~AVGSDLDisplayEngine();
        virtual void init(int width, int height, bool isFullscreen);
        virtual void teardown();

        virtual void setClipRect(const PLRect& rc);
        virtual void render(PLBmp * pBmp, const PLPoint& pos);
        virtual void swapBuffers();

		virtual PLBmp * createSurface();

    private:
        int m_Width;
        int m_Height;
        bool m_IsFullscreen;

        PLSDLBmp * m_pScreen;
};

#endif //_AVGSDLDisplayEngine_H_
