//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2011 Ulrich von Zadow
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

#ifndef _GDKDisplayEngine_H_
#define _GDKDisplayEngine_H_

#include "../api.h"
#include "IInputDevice.h"
#include "DisplayEngine.h"

#include "../graphics/GLConfig.h"
#include "../graphics/Bitmap.h"
#include "../graphics/Pixel32.h"
#include "../graphics/OGLHelper.h"
#include "../graphics/FBO.h"

#include <string>
#include <vector>

#include <gdk/gdk.h>
#include <gdk/gdkx.h>

#include "TouchEvent.h"
#include "TouchStatus.h"
namespace avg {

class XInputMTInputDevice;
class MouseEvent;
typedef boost::shared_ptr<class MouseEvent> MouseEventPtr;
class GLContext;

class AVG_API GDKDisplayEngine: public DisplayEngine, public IInputDevice
{
    public:
        GDKDisplayEngine();
        virtual ~GDKDisplayEngine();
        virtual void init(const DisplayParams& dp, GLConfig glConfig);

        // From DisplayEngine
        virtual void teardown();
        virtual float getRefreshRate();
        virtual void setGamma(float red, float green, float blue);
        virtual void setMousePos(const IntPoint& pos);
        virtual int getKeyModifierState() const;

        virtual IntPoint getSize();

        virtual void showCursor(bool bShow);
        virtual BitmapPtr screenshot(int buffer=0);

        // From IInputDevice
        virtual std::vector<EventPtr> pollEvents();
        void setXIMTInputDevice(XInputMTInputDevice* pInputDevice);

        const IntPoint& getWindowSize() const;
        bool isFullscreen() const;
        IntPoint getScreenResolution();
        float getPixelsPerMM();
        glm::vec2 getPhysicalScreenDimensions();
        void assumePixelsPerMM(float ppmm);
        virtual void swapBuffers();
        void setCursor(GdkPixbuf *pixbuf, int x, int y);

        GdkDisplay* getDisplay();
        void enableGDKMultitouchHandling(bool value);

    private:
        void initTranslationTable();
        void calcScreenDimensions(float dotsPerMM=0);

        bool internalSetGamma(float red, float green, float blue);

        EventPtr createMouseEvent
                (Event::Type Type, const GdkEvent & GDKEvent, long Button);
        EventPtr createMouseButtonEvent(Event::Type Type, const GdkEvent & GDKEvent);
        EventPtr createMouseWheelEvent(Event::Type Type, const GdkEvent & GDKEvent);
        EventPtr createKeyEvent(Event::Type Type, const GdkEvent & GDKEvent);
        TouchEventPtr createTouchEvent(int ID, Event::Type Type, const GdkEvent & GDKEvent);

        IntPoint m_Size;
        bool m_bIsFullscreen;
        IntPoint m_WindowSize;
        IntPoint m_ScreenResolution;
        float m_PPMM;

        GdkWindow* m_pScreen;       //fenster
        GdkScreen* m_screen;

        static void calcRefreshRate();
        static float s_RefreshRate;

        // Event handling.
        bool m_bMouseOverApp;
        MouseEventPtr m_pLastMouseEvent;
        int m_NumMouseButtonsDown;
        static std::vector<long> KeyCodeTranslationTable;
        XInputMTInputDevice * m_pXIMTInputDevice;

        GLContext* m_pGLContext;
        GdkCursor* m_noneCursor;
        GdkCursor* m_cursor;

        int m_touchID;
        bool m_multitouch;

        float m_Gamma[3];
};

typedef boost::shared_ptr<GDKDisplayEngine> GDKDisplayEnginePtr;

}

#endif
