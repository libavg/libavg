
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

#ifndef _DisplayParams_H_
#define _DisplayParams_H_

#include "../api.h"

#include "WindowParams.h"

#include "../base/GLMHelper.h"

#include <vector>

namespace avg {

class AVG_API DisplayParams {
public:
    DisplayParams();
    virtual ~DisplayParams();

    void calcWindowSizes();
    void setResolution(bool bFullscreen, int width, int height, int bpp);
    void setFullscreen(bool bFullscreen);
    void setBPP(int bpp);
    void setGamma(float red, float green, float blue);
    void setFramerate(float framerate, int vbRate);
    void setShowCursor(bool bShow);
    void resetWindows();

    bool isFullscreen() const;
    int getBPP() const;
    bool isCursorVisible() const;
    int getVBRate() const;
    float getFramerate() const;
    int getNumWindows() const;
    WindowParams& getWindowParams(int i);
    const WindowParams& getWindowParams(int i) const;
    const float getGamma(int i) const;

    void dump() const;

private:
    bool m_bFullscreen;
    int m_BPP;
    bool m_bShowCursor;
    int m_VBRate;
    float m_Framerate;

    float m_Gamma[3];

    std::vector<WindowParams> m_Windows;
};

}

#endif
