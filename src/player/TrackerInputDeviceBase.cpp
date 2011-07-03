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

#include "TrackerInputDeviceBase.h"
#include "TrackerTouchStatus.h"
#include "TouchEvent.h"

#include "AVGNode.h"


namespace avg {

bool operator < (const BlobDistEntryPtr& e1, const BlobDistEntryPtr& e2){
    return e1->m_Dist > e2->m_Dist;
}

TrackerInputDeviceBase::TrackerInputDeviceBase(const std::string& name):IInputDevice(name)
{
    m_TrackerConfig.load();
    m_pDeDistort = m_TrackerConfig.getTransform();
}

void TrackerInputDeviceBase::pollEventType(std::vector<EventPtr>& result,
        TouchStatusMap& Events,  CursorEvent::Source source)
{
    EventPtr pEvent;
    for (TouchStatusMap::iterator it = Events.begin(); it!= Events.end();) {
        TrackerTouchStatusPtr pTouchStatus = (*it).second;
        pEvent = pTouchStatus->pollEvent();
        if (pEvent) {
            result.push_back(pEvent);
            if (pEvent->getType() == Event::CURSORUP) {
                Events.erase(it++);
            } else {
                ++it;
            }
        } else {
            ++it;
        }
    }
};

void TrackerInputDeviceBase::trackBlobIDs(BlobVectorPtr pNewBlobs, long long time,
        bool bTouch)
{
    TouchStatusMap * pEvents;
    std::string sConfigPath;
    Event::Source source;
    if (bTouch) {
        sConfigPath = "/tracker/touch/";
        pEvents = &m_TouchEvents;
        source = Event::TOUCH;
    } else {
        sConfigPath = "/tracker/track/";
        pEvents = &m_TrackEvents;
        source = Event::TRACK;
    }
    BlobVector oldBlobs;
    for (TouchStatusMap::iterator it = pEvents->begin(); it != pEvents->end(); ++it) {
        (*it).second->setStale();
        oldBlobs.push_back((*it).first);
    }
    // Create a heap that contains all distances of old to new blobs < MaxDist
    double MaxDist = m_TrackerConfig.getDoubleParam(sConfigPath+"similarity/@value");
    double MaxDistSquared = MaxDist*MaxDist;
    std::priority_queue<BlobDistEntryPtr> distHeap;
    for (BlobVector::iterator it = pNewBlobs->begin(); it != pNewBlobs->end(); ++it) {
        BlobPtr pNewBlob = *it;
        for(BlobVector::iterator it2 = oldBlobs.begin(); it2 != oldBlobs.end(); ++it2) { 
            BlobPtr pOldBlob = *it2;
            double distSquared = calcDistSquared(pNewBlob->getCenter(),
                    pOldBlob->getEstimatedNextCenter());
            if (distSquared <= MaxDistSquared) {
                BlobDistEntryPtr pEntry = BlobDistEntryPtr(
                        new BlobDistEntry(distSquared, pNewBlob, pOldBlob));
                distHeap.push(pEntry);
            }
        }
    }
    // Match up the closest blobs.
    std::set<BlobPtr> matchedNewBlobs;
    std::set<BlobPtr> matchedOldBlobs;
    while (!distHeap.empty()) {
        BlobDistEntryPtr pEntry = distHeap.top();
        distHeap.pop();
        if (matchedNewBlobs.find(pEntry->m_pNewBlob) == matchedNewBlobs.end() &&
            matchedOldBlobs.find(pEntry->m_pOldBlob) == matchedOldBlobs.end())
        {
            // Found a pair of matched blobs.
            BlobPtr pNewBlob = pEntry->m_pNewBlob; 
            BlobPtr pOldBlob = pEntry->m_pOldBlob; 
            matchedNewBlobs.insert(pNewBlob);
            matchedOldBlobs.insert(pOldBlob);
            AVG_ASSERT (pEvents->find(pOldBlob) != pEvents->end());
            TrackerTouchStatusPtr pTouchStatus;
            pTouchStatus = pEvents->find(pOldBlob)->second;
            // Make sure we don't discard any events that have related info.
            bool bKeepAllEvents = pNewBlob->getFirstRelated() && !bTouch;
            pTouchStatus->blobChanged(pNewBlob, time, bKeepAllEvents);
            pNewBlob->calcNextCenter(pOldBlob->getCenter());
            // Update the mapping.
            (*pEvents)[pNewBlob] = pTouchStatus;
            pEvents->erase(pOldBlob);
        }
    }
    // Blobs have been matched. Left-overs are new blobs.
    for (BlobVector::iterator it = pNewBlobs->begin(); it != pNewBlobs->end(); ++it) {
        if (matchedNewBlobs.find(*it) == matchedNewBlobs.end()) {
            TrackerTouchStatusPtr pTouchStatus = TrackerTouchStatusPtr(
                    new TrackerTouchStatus(*it, time, m_pDeDistort, m_DisplayROI, 
                            source));
            (*pEvents)[(*it)] = pTouchStatus;
        }
    }

    // All event streams that are still stale haven't been updated: blob is gone, 
    // set the sentinel for this.
    for(TouchStatusMap::iterator it=pEvents->begin(); it!=pEvents->end(); ++it) {
        if ((*it).second->isStale()) {
            (*it).second->blobGone();
        }
    }
};

} //End namespace avg
