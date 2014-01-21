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

#include "WindowParams.h"

#include "../base/Exception.h"

#include <iostream>

using namespace std;

namespace avg {

WindowParams::WindowParams()
    : m_Pos(-1, -1),
      m_Size(0, 0),
      m_Viewport(0, 0, 0, 0),
      m_DisplayServer(0),
      m_bHasWindowFrame(true)
{ 
}

WindowParams::~WindowParams()
{
}

void WindowParams::calcSize()
{
    float aspectRatio = float(m_Viewport.width())/float(m_Viewport.height());
    IntPoint windowSize;
    IntPoint viewportSize = m_Viewport.size();
    if (m_Size == IntPoint(0, 0)) {
        windowSize = viewportSize;
    } else if (m_Size.x == 0) {
        windowSize.x = int(viewportSize.y*aspectRatio);
        windowSize.y = viewportSize.y;
    } else {
        windowSize.x = viewportSize.x;
        windowSize.y = int(viewportSize.x/aspectRatio);
    }
    AVG_ASSERT(windowSize.x != 0 && windowSize.y != 0);
    m_Size = windowSize;
}

void WindowParams::dump() const
{
    cerr << "  WindowParams: " << endl;
    cerr << "    pos: " << m_Pos << endl;
    cerr << "    size: " << m_Size << endl;
    cerr << "    viewport: " << m_Viewport << endl;
    cerr << "    has window frame: " << m_bHasWindowFrame << endl;
}

}
