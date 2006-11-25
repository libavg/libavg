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

#ifndef _Tracker_H_
#define _Tracker_H_

#include "Camera.h"
#include "TrackerThread.h"

#include "../graphics/Bitmap.h"

#include <boost/thread.hpp>

#include <string>
#include <map>

namespace avg {

class Tracker
{
    public:
        Tracker(std::string sDevice, double FrameRate, std::string sMode);
        virtual ~Tracker();

        // More parameters possible: Barrel/pincushion, history length,...
        void setThreshold(int Threshold);

        const BitmapPtr getImage(TrackerImageID ImageID) const;
        TouchInfoListPtr getTouches();

    private:
        boost::thread* m_pThread;

        TouchInfoListPtr m_pTouchInfoList;
        BitmapPtr m_pBitmaps[3];
        MutexPtr m_pMutex;
        // We'll need a Command Queue too, at least for stop & threshold, possibly for 
        // other params.
};

}

#endif

