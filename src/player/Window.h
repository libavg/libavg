
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
#include "DisplayParams.h"

#include "../base/Rect.h"
#include "../graphics/GLConfig.h"

#include <boost/shared_ptr.hpp>
#include <string>
#include <vector>

union SDL_Event;

namespace avg {

class XInputMTInputDevice;
class MouseEvent;
typedef boost::shared_ptr<class MouseEvent> MouseEventPtr;
class Bitmap;
typedef boost::shared_ptr<class Bitmap> BitmapPtr;
class GLContext;

class AVG_API Window
{
    public:
        Window(const DisplayParams& dp, GLConfig glConfig);
        virtual ~Window();

        void setTitle(const std::string& sTitle);
        virtual BitmapPtr screenshot(int buffer=0);

        const IntPoint& getSize() const;
        bool isFullscreen() const;
        virtual void swapBuffers();

    private:
        bool internalSetGamma(float red, float green, float blue);

        bool m_bIsFullscreen;
        IntPoint m_Size;
        IntRect m_Viewport;

        GLContext* m_pGLContext;
};

typedef boost::shared_ptr<Window> WindowPtr;

}

#endif
