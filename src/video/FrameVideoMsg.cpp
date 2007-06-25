//
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

#include "FrameVideoMsg.h"

#include <iostream>

using namespace std;

namespace avg {

FrameVideoMsg::FrameVideoMsg(std::vector<BitmapPtr> pBmps, long long FrameTime, 
        bool bSeekDone)
{
    m_pBmps = pBmps;
    m_FrameTime = FrameTime;
    m_bSeekDone = bSeekDone;
}

FrameVideoMsg::~FrameVideoMsg()
{
}

BitmapPtr FrameVideoMsg::getBitmap(int i)
{
    return m_pBmps[i];
}
        
long long FrameVideoMsg::getFrameTime()
{
    return m_FrameTime;
}

bool FrameVideoMsg::isSeekDone()
{
    return m_bSeekDone;
}

}

