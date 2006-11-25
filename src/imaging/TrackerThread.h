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

#ifndef _TrackerThread_H_
#define _TrackerThread_H_

#include "Camera.h"

#include "../graphics/Bitmap.h"

#include <boost/thread.hpp>

#include <string>
#include <list>

namespace avg {

struct TouchInfo {
    int m_ID;
    DPoint m_Center;
    int m_Area;
    // More to follow?
};

typedef std::list<TouchInfo> TouchInfoList;
typedef boost::shared_ptr<TouchInfoList> TouchInfoListPtr;

typedef enum {
        TRACKER_IMG_CAMERA,
        TRACKER_IMG_HISTORY,
        TRACKER_IMG_NOHISTORY,
        TRACKER_IMG_COMPONENTS,
        NUM_TRACKER_IMAGES
} TrackerImageID;

typedef boost::shared_ptr<boost::mutex> MutexPtr;

class TrackerThread
{
    public:
        TrackerThread(std::string sDevice, double FrameRate, std::string sMode, 
                TouchInfoListPtr pTouchInfoList, BitmapPtr ppBitmaps[NUM_TRACKER_IMAGES],
                MutexPtr pMutex);
        virtual ~TrackerThread();

        void operator()();

    private:
        void open();
        void close();
        void track();
        void checkMessages();
        void calcHistory();

        std::string m_sDevice;
        double m_FrameRate;
        std::string m_sMode;

        TouchInfoListPtr m_pTouchInfoList;
        BitmapPtr m_pBitmaps[NUM_TRACKER_IMAGES];
        MutexPtr m_pMutex;
        BitmapPtr m_pHistoryBmp;

        CameraPtr  m_pCamera;

        bool m_bShouldStop;
};

}

#endif

