
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

struct AVG_API DisplayParams {
    DisplayParams();
    virtual ~DisplayParams();

    void resetWindows();
    void dump() const;

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
