//
//  libavg - Media Playback Engine.
//  Copyright (C) 2003-2021 Ulrich von Zadow
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
#include "InputDevice.h"

#include "../graphics/GLConfig.h"


#include <boost/shared_ptr.hpp>

#include <string>
#include <vector>

namespace avg {

class MouseEvent;
typedef boost::shared_ptr<class MouseEvent> MouseEventPtr;
class Window;
typedef boost::shared_ptr<class Window> WindowPtr;
class Bitmap;
typedef boost::shared_ptr<class Bitmap> BitmapPtr;
class GLContext;
class DisplayParams;

class AVG_API DisplayEngine: public InputDevice
{   
    public:
        static void initSDL();
        static void quitSDL();

        DisplayEngine();
        virtual ~DisplayEngine();
        virtual void init(const DisplayParams& dp, GLConfig glConfig);
        virtual void teardown();
        void initRender();
        void deinitRender();
        void setFramerate(float rate);
        float getFramerate();
        float getEffectiveFramerate();
        void setVBlankRate(int rate);
        bool wasFrameLate();
        void setGamma(float Red, float Green, float Blue);
        void setMousePos(const IntPoint& pos);
        int getKeyModifierState() const;
    
        unsigned getNumWindows() const;
        const WindowPtr getWindow(unsigned i) const;

        void endFrame();
        void frameWait();
        void swapBuffers();
        void checkJitter();
        long long getDisplayTime();

        const IntPoint& getSize() const;
        IntPoint getWindowSize() const;
        bool isFullscreen() const;

        void showCursor(bool bShow);

        BitmapPtr screenshot(int buffer=0);

        // From InputDevice
        std::vector<EventPtr> pollEvents();

    private:
        std::vector<WindowPtr> m_pWindows;
        IntPoint m_Size;
        std::string m_sWindowTitle;

        float m_Gamma[3];
        int m_NumFrames;
        int m_FramesTooLate;
        long long m_StartTime;
        long long m_TimeSpentWaiting;

        // Per-Frame timings.
        long long m_LastFrameTime;
        long long m_FrameWaitStartTime;
        long long m_TargetTime;
        int m_VBRate;
        float m_Framerate;
        bool m_bInitialized;
        bool m_bFrameLate;

        float m_EffFramerate;
};

typedef boost::shared_ptr<DisplayEngine> DisplayEnginePtr;

}

#endif
