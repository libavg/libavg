//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2008 Ulrich von Zadow
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

#ifndef _TrackerTouchStatus_H_
#define _TrackerTouchStatus_H_

#include "../api.h"
#include "Event.h"
#include "TouchStatus.h"

#include "../base/Point.h"

#include "../imaging/Blob.h"
#include "../imaging/DeDistort.h"

#include <string>

namespace avg {

class AVG_API TrackerTouchStatus: public TouchStatus
{
    public:
        TrackerTouchStatus(BlobPtr pFirstBlob, long long time, DeDistortPtr pDeDistort, 
                const DRect& displayROI, Event::Source source);
        virtual ~TrackerTouchStatus();
        void blobChanged(BlobPtr pNewBlob, long long time, bool bKeepEvent);
        void blobGone();
        void setStale();
        bool isStale();

    private:
        TouchEventPtr createEvent(Event::Source source, Event::Type type, int id, 
                BlobPtr pBlob, long long time, DeDistortPtr pDeDistort, 
                const DRect& displayROI);
        TouchEventPtr createEvent(Event::Type type, BlobPtr pBlob, long long time);

        Event::Source m_Source;
        DeDistortPtr m_pDeDistort;
        DRect m_DisplayROI;
        bool m_Stale;
        bool m_bGone;
        int m_ID;
        BlobPtr m_pBlob;
        long long m_LastTime;
        DPoint m_LastCenter;

        static int s_LastID;
};

}

#endif
