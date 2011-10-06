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

#include "VideoDecoder.h"
#include "../base/Exception.h"

namespace avg {

FrameAvailableCode VideoDecoder::renderToBmp(BitmapPtr pBmp, double timeWanted)
{
    std::vector<BitmapPtr> pBmps;
    pBmps.push_back(pBmp);
    return renderToBmps(pBmps, timeWanted);
}

FrameAvailableCode VideoDecoder::renderToVDPAU(vdpau_render_state** ppRenderState)
{
    AVG_ASSERT(false);
    return FA_NEW_FRAME; // Silence compiler warning.
}

}


