//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2006 Ulrich von Zadow
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

#include "DisplayParams.h"

namespace avg {

DisplayParams::DisplayParams()
    : m_Width(0),
      m_Height(0),
      m_bFullscreen(false),
      m_BPP(24),
      m_WindowWidth(0),
      m_WindowHeight(0),
      m_bShowCursor(true),
      m_VBRate(1),
      m_Framerate(0)
{ 
    m_Gamma[0] = -1.0;
    m_Gamma[1] = -1.0;
    m_Gamma[2] = -1.0;
}
/*
DisplayParams::DisplayParams(int Width, int Height, bool bFullscreen, int bpp, 
            int WindowWidth, int WindowHeight)
    : m_bShowCursor(true),
      m_VBlankRate(1),
      m_Framerate(0)
{
    m_Width = Width;
    m_Height = Height;
    m_bFullscreen = bFullscreen;
    m_BPP = BPP;
    m_WindowWidth = WindowWidth;
    m_WindowHeight = WindowHeight;
    m_Gamma[0] = -1.0;
    m_Gamma[1] = -1.0;
    m_Gamma[2] = -1.0;
}
*/
}

