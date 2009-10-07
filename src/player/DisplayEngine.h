
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2008 Ulrich von Zadow
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  Current versions can be found at www.libavg.de
//

#ifndef _DisplayEngine_H_
#define _DisplayEngine_H_

#include "../api.h"
#include "Node.h"
#include "DisplayParams.h"

#include "../video/IVideoDecoder.h"

#include "../graphics/Pixel32.h"
#include "../graphics/Bitmap.h"

#include "../base/Rect.h"

#include <string>

namespace avg {

class Region;
class OGLTiledSurface;

class AVG_API DisplayEngine
{   
    public:
        DisplayEngine();
        virtual ~DisplayEngine();
        virtual void init(const DisplayParams& DP) = 0;
        virtual void teardown() = 0;
        void initRender();
        void deinitRender();
        void setFramerate(double rate);
        double getFramerate();
        double getEffectiveFramerate();
        bool setVBlankRate(int rate);
        bool wasFrameLate();
        virtual double getRefreshRate() = 0;
        virtual void setGamma(double Red, double Green, double Blue) = 0;
        virtual void setMousePos(const IntPoint& pos) = 0;

        virtual void render(AVGNodePtr pRootNode) = 0;
        void frameWait();
        long long getDisplayTime();
        
        virtual bool pushClipRect(const DRect& rc) = 0;
        virtual void popClipRect() = 0;
        virtual const DRect& getClipRect() = 0;
        virtual void pushTransform(const DPoint& translate, double angle, const DPoint& pivot) = 0;
        virtual void popTransform() = 0;

        virtual int getWidth() = 0;
        virtual int getHeight() = 0;
        virtual int getBPP() = 0;

        // True if the bpp given are supported.
        virtual bool supportsBpp(int bpp) = 0; 
        // True if pixel order is RGB; BGR otherwise.
        virtual bool hasRGBOrdering() = 0; 
        virtual bool isUsingShaders() const = 0; 
        virtual void showCursor (bool bShow) = 0;

        virtual BitmapPtr screenshot () = 0;

        enum BlendMode {BLEND_BLEND, BLEND_ADD, BLEND_MIN, BLEND_MAX};
        static BlendMode stringToBlendMode(const std::string& s);

    protected:
        void checkJitter();
        
    private:
        virtual bool initVBlank(int rate) = 0;
        virtual bool vbWait(int rate) = 0;
        
        int m_NumFrames;
        int m_FramesTooLate;
        long long m_FrameWaitStartTime;
        long long m_TimeSpentWaiting;
        long long m_StartTime;
        long long m_TargetTime;     // in microseconds.
        long long m_LastFrameTime;  // in microseconds.
        long long m_LastVideoFrameTime;  // measured by counting refreshes, in microseconds.
        int m_VBRate;
        double m_Framerate;
        bool m_bInitialized;
        bool m_bFrameLate;

        long long m_StartFramerateCalcTime;
        double m_EffFramerate;
};

}

#endif //_DisplayEngine_H_
