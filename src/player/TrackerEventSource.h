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

#ifndef _TrackerEventSource_H_
#define _TrackerEventSource_H_

#include "Event.h"
#include "IEventSource.h"
#include "../graphics/Bitmap.h"
#include "../imaging/Tracker.h"
#include "../imaging/ConnectedComps.h"

#include <string>
#include <map>
#include <list>
#include <vector>
#include <utility>
namespace avg {

class EventStream;
typedef boost::shared_ptr<EventStream> EventStreamPtr;
typedef std::map<BlobPtr, EventStreamPtr> EventMap;
class TrackerEventSource:public IBlobTarget, IEventSource
{
    public:
        TrackerEventSource(std::string sDevice, double FrameRate, std::string sMode);
        virtual ~TrackerEventSource();

        // More parameters possible: Barrel/pincushion, history length,...
        void setConfig(TrackerConfig tracker_config);

        Bitmap * getImage(TrackerImageID ImageID) const;
        std::vector<Event *> pollEvents();//main thread

        /*implement IBlobTarget*/
        virtual void update(BlobListPtr new_blobs);//tracker thread

    protected:
        bool isfinger(BlobPtr blob);
        BlobPtr matchblob(BlobPtr new_blob, BlobListPtr old_blobs, double threshold);
    private:
        Tracker m_Tracker;
        TrackerConfig m_TrackerConfig;
        EventMap m_Events;
        MutexPtr m_pMutex;
        // We'll need a Command Queue too, at leas for threshold, possibly for 
        // other params.
};

typedef boost::shared_ptr<TrackerEventSource> TrackerEventSourcePtr;

}

#endif

