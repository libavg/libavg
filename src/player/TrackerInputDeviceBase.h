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

#ifndef _TrackerInputDeviceBase_H_
#define _TrackerInputDeviceBase_H_

#include "../api.h"
#include "CursorEvent.h"
#include "IInputDevice.h"
#include "MultitouchInputDevice.h"

#include "../base/Logger.h"
#include "../imaging/Blob.h"
#include "../imaging/TrackerConfig.h"

#include <map>
#include <vector>
#include <string>
#include <queue>
#include <set>

namespace avg{

typedef enum {
    TOUCH_BLOB,
    TRACK_BLOB
} BlobType;

// Temporary structure to be put into heap of blob distances. Used only in 
// trackBlobIDs.
struct BlobDistEntry {
    BlobDistEntry(double dist, BlobPtr pNewBlob, BlobPtr pOldBlob) 
        : m_Dist(dist),
          m_pNewBlob(pNewBlob),
          m_pOldBlob(pOldBlob)
    {
    }

    double m_Dist;
    BlobPtr m_pNewBlob;
    BlobPtr m_pOldBlob;
};
typedef boost::shared_ptr<struct BlobDistEntry> BlobDistEntryPtr;

// The heap is sorted by least distance, so this operator does the
// _opposite_ of what is expected!
bool operator < (const BlobDistEntryPtr& e1, const BlobDistEntryPtr& e2);



class TrackerTouchStatus;
typedef boost::shared_ptr<TrackerTouchStatus> TrackerTouchStatusPtr;

class AVG_API TrackerInputDeviceBase: public IInputDevice
{
    typedef std::map<BlobPtr, TrackerTouchStatusPtr> TouchStatusMap;

    public:
        TrackerInputDeviceBase(const std::string&
            name=EXTRACT_INPUTDEVICE_CLASSNAME(TrackerInputDeviceBase));
        virtual ~TrackerInputDeviceBase() {};

        // Note that this function is called by TrackerThread in it's own thread!
        virtual void updateBlobs(BlobVectorPtr pBlobs, BlobType type, long long time) = 0;
        virtual std::vector<EventPtr> pollEvents() = 0;

    protected:
        void pollEventType(std::vector<EventPtr>& result, TouchStatusMap& Events,
                CursorEvent::Source source);
        // Used by tracker thread
        void trackBlobIDs(BlobVectorPtr pNewBlobs, long long time,
                bool bTouch);

        DeDistortPtr m_pDeDistort;
        DRect m_DisplayROI;
        // Used by both threads
        TouchStatusMap m_TouchEvents;
        TouchStatusMap m_TrackEvents;
        TrackerConfig m_TrackerConfig;

    private:

};//End class

}//End namespace avg

#endif
