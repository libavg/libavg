
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

#ifndef _Window_H_
#define _Window_H_

#include "../api.h"
#include "WindowParams.h"
#include "DisplayParams.h"
#include "Event.h"

#include "../base/Rect.h"

#include <boost/shared_ptr.hpp>
#include <string>
#ifdef _WIN32
#include <Windows.h>
#endif
#ifdef __linux__
#include <X11/Xlib.h>
#endif

namespace avg {

class Bitmap;
typedef boost::shared_ptr<class Bitmap> BitmapPtr;
class GLContext;

class AVG_API Window
{
    public:
        Window(const WindowParams& wp, bool bIsFullscreen);
        virtual ~Window();

        virtual void setTitle(const std::string& sTitle) = 0;
        virtual void swapBuffers() const = 0;
        BitmapPtr screenshot(int buffer=0);

        const IntPoint& getPos() const;
        const IntPoint& getSize() const;
        const IntRect& getViewport() const;
        bool isFullscreen() const;
        GLContext* getGLContext() const;
        
        virtual std::vector<EventPtr> pollEvents() = 0;

        virtual void setGamma(float red, float green, float blue) {};
#ifdef _WIN32
        virtual HWND getWinHWnd() = 0;
#endif
#ifdef __linux__
        virtual ::Display* getX11Display() { return 0;};
        virtual ::Window getX11Window() { return 0;};
#endif

    protected:
        void setGLContext(GLContext* pGLContext);

    private:
        bool m_bIsFullscreen;
        IntPoint m_Pos;
        IntPoint m_Size;
        IntRect m_Viewport;

        GLContext* m_pGLContext;
};

typedef boost::shared_ptr<Window> WindowPtr;

}

#endif
