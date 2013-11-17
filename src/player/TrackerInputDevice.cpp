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
//  Original author of this file is igor@c-base.org
//

#include "TrackerInputDevice.h"
#include "TouchEvent.h"
#include "TrackerTouchStatus.h"

#include "../base/Logger.h"
#include "../base/ObjectCounter.h"
#include "../base/ScopeTimer.h"
#include "../base/Exception.h"

#include "../graphics/HistoryPreProcessor.h"
#include "../graphics/Filterfill.h"
#include "../graphics/Pixel8.h"

#include "../imaging/DeDistort.h"
#include "../imaging/CoordTransformer.h"

#include "../glm/gtx/norm.hpp"

#include "Player.h"
#include "AVGNode.h"

#include <boost/thread/thread.hpp>
#include <boost/bind.hpp>

#include <map>
#include <list>
#include <vector>
#include <queue>
#include <set>
#include <iostream>

using namespace std;

namespace avg {

TrackerInputDevice::TrackerInputDevice()
    : IInputDevice(EXTRACT_INPUTDEVICE_CLASSNAME(TrackerInputDevice)),
      m_pTrackerThread(0),
      m_bSubtractHistory(true),
      m_pCalibrator(0)
{
    ObjectCounter::get()->incRef(&typeid(*this));

    m_TrackerConfig.load();

    string sDriver = m_TrackerConfig.getParam("/camera/driver/@value");
    string sDevice = m_TrackerConfig.getParam("/camera/device/@value");
    bool bFW800 = m_TrackerConfig.getBoolParam("/camera/fw800/@value");
    IntPoint captureSize(m_TrackerConfig.getPointParam("/camera/size/"));
    string sCaptureFormat = m_TrackerConfig.getParam("/camera/format/@value");
    float frameRate = m_TrackerConfig.getFloatParam("/camera/framerate/@value");

    PixelFormat camPF = stringToPixelFormat(sCaptureFormat);
    if (camPF == NO_PIXELFORMAT) {
        throw Exception(AVG_ERR_INVALID_ARGS,
                "Unknown camera pixel format "+sCaptureFormat+".");
    }
    
    AVG_TRACE(Logger::category::CONFIG, Logger::severity::INFO,
            "Trying to create a Tracker for " << sDriver
            << " Camera: " << sDevice << " Size: " << captureSize << "format: "
            << sCaptureFormat);
    m_pCamera = createCamera(sDriver, sDevice, -1, bFW800, captureSize, camPF, I8, 
            frameRate);
    AVG_TRACE(Logger::category::CONFIG, Logger::severity::INFO,
            "Got Camera " << m_pCamera->getDevice() << " from driver: " 
            << m_pCamera->getDriverName());

    IntPoint imgSize = m_pCamera->getImgSize();
    m_pBitmaps[0] = BitmapPtr(new Bitmap(imgSize, I8));
    m_pMutex = MutexPtr(new boost::mutex);
    m_pCmdQueue = TrackerThread::CQueuePtr(new TrackerThread::CQueue);
    m_pDeDistort = m_TrackerConfig.getTransform();
    try {
        m_ActiveDisplaySize = IntPoint(
                m_TrackerConfig.getPointParam("/transform/activedisplaysize/"));
    } catch (Exception) {
        m_ActiveDisplaySize = IntPoint(Player::get()->getRootNode()->getSize());
    }
    try {
        m_DisplayROI = m_TrackerConfig.getRectParam("/transform/displayroi/");
    } catch (Exception) {
        m_DisplayROI = FRect(glm::vec2(0,0), glm::vec2(m_ActiveDisplaySize));
    }

    IntRect roi = m_pDeDistort->getActiveBlobArea(m_DisplayROI);
    if (roi.tl.x < 0 || roi.tl.y < 0 || 
            roi.br.x > imgSize.x || roi.br.y > imgSize.y) 
    {
        AVG_LOG_ERROR("Impossible tracker configuration: Region of interest is " 
                << roi << ", camera image size is " << imgSize << ". Aborting.");
        exit(5);
    }
    m_InitialROI = roi;
    createBitmaps(roi);
    setDebugImages(false, false);

    try {
        m_bFindFingertips = m_TrackerConfig.getBoolParam(
                "/tracker/findfingertips/@value");
    } catch(Exception) {
        m_bFindFingertips = false;
    }

}

TrackerInputDevice::~TrackerInputDevice()
{
    m_pCmdQueue->pushCmd(boost::bind(&TrackerThread::stop, _1));
    if (m_pTrackerThread) {
        m_pTrackerThread->join();
        delete m_pTrackerThread;
    }            
    ObjectCounter::get()->decRef(&typeid(*this));
}

void TrackerInputDevice::start()
{
    m_pTrackerThread = new boost::thread(
            TrackerThread(
                m_InitialROI,
                m_pCamera,
                m_pBitmaps, 
                m_pMutex,
                *m_pCmdQueue,
                this,
                m_bSubtractHistory,
                m_TrackerConfig
                )
            );
    setConfig();
}

void TrackerInputDevice::setParam(const string& sElement, const string& sValue)
{
    string sOldParamVal = m_TrackerConfig.getParam(sElement);
    m_TrackerConfig.setParam(sElement, sValue);

    // Test if active area is outside camera.
    FRect area = m_pDeDistort->getActiveBlobArea(m_DisplayROI);
    glm::vec2 size = m_TrackerConfig.getPointParam("/camera/size/");
    int prescale = m_TrackerConfig.getIntParam("/tracker/prescale/@value");
    if (area.br.x > size.x/prescale || area.br.y > size.y/prescale ||
        area.tl.x < 0 || area.tl.y < 0)
    {
        m_TrackerConfig.setParam(sElement, sOldParamVal);
    } else {
        setConfig();
    }
//        m_TrackerConfig.dump();
}

string TrackerInputDevice::getParam(const string& sElement)
{
    return m_TrackerConfig.getParam(sElement);
}
   
void TrackerInputDevice::setDebugImages(bool bImg, bool bFinger)
{
    m_pCmdQueue->pushCmd(boost::bind(&TrackerThread::setDebugImages, _1, bImg, 
            bFinger));
}

void TrackerInputDevice::resetHistory()
{
    m_pCmdQueue->pushCmd(boost::bind(&TrackerThread::resetHistory, _1));
}

void TrackerInputDevice::saveConfig()
{
    m_TrackerConfig.save();
}

void TrackerInputDevice::setConfig()
{
    m_pDeDistort = m_TrackerConfig.getTransform();
    FRect area = m_pDeDistort->getActiveBlobArea(m_DisplayROI);
    createBitmaps(area);
    m_pCmdQueue->pushCmd(boost::bind(&TrackerThread::setConfig, _1, m_TrackerConfig, 
            area, m_pBitmaps));
}

void TrackerInputDevice::createBitmaps(const IntRect& area)
{
    lock_guard lock(*m_pMutex);
    for (int i=1; i<NUM_TRACKER_IMAGES; i++) {
        switch (i) {
            case TRACKER_IMG_HISTOGRAM:
                m_pBitmaps[TRACKER_IMG_HISTOGRAM] = 
                        BitmapPtr(new Bitmap(IntPoint(256, 256), I8));
                FilterFill<Pixel8>(Pixel8(0)).
                        applyInPlace(m_pBitmaps[TRACKER_IMG_HISTOGRAM]);
                break;
            case TRACKER_IMG_FINGERS:
                m_pBitmaps[TRACKER_IMG_FINGERS] = 
                        BitmapPtr(new Bitmap(area.size(), B8G8R8A8));
                FilterFill<Pixel32>(Pixel32(0,0,0,0)).
                        applyInPlace(m_pBitmaps[TRACKER_IMG_FINGERS]);
                break;
            default:
                m_pBitmaps[i] = BitmapPtr(new Bitmap(area.size(), I8));
                FilterFill<Pixel8>(Pixel8(0)).applyInPlace(m_pBitmaps[i]);
        }
    }
}

Bitmap * TrackerInputDevice::getImage(TrackerImageID imageID) const
{
    lock_guard lock(*m_pMutex);
    return new Bitmap(*m_pBitmaps[imageID]);
}

glm::vec2 TrackerInputDevice::getDisplayROIPos() const
{
    return m_DisplayROI.tl;
}

glm::vec2 TrackerInputDevice::getDisplayROISize() const
{
    return m_DisplayROI.size();
}

static ProfilingZoneID ProfilingZoneCalcTrack("trackBlobIDs(track)");
static ProfilingZoneID ProfilingZoneCalcTouch("trackBlobIDs(touch)");

void TrackerInputDevice::update(BlobVectorPtr pTrackBlobs, 
        BlobVectorPtr pTouchBlobs, long long time)
{
    if (pTrackBlobs) {
        ScopeTimer Timer(ProfilingZoneCalcTrack);
        trackBlobIDs(pTrackBlobs, time, false);
    }
    if (pTouchBlobs) {
        ScopeTimer Timer(ProfilingZoneCalcTouch);
        trackBlobIDs(pTouchBlobs, time, true);
    }
}

// Temporary structure to be put into heap of blob distances. Used only in 
// trackBlobIDs.
struct BlobDistEntry {
    BlobDistEntry(float dist, BlobPtr pNewBlob, BlobPtr pOldBlob) 
        : m_Dist(dist),
          m_pNewBlob(pNewBlob),
          m_pOldBlob(pOldBlob)
    {
    }

    float m_Dist;
    BlobPtr m_pNewBlob;
    BlobPtr m_pOldBlob;
};
typedef boost::shared_ptr<struct BlobDistEntry> BlobDistEntryPtr;

// The heap is sorted by least distance, so this operator does the
// _opposite_ of what is expected!
bool operator < (const BlobDistEntryPtr& e1, const BlobDistEntryPtr& e2) 
{
    return e1->m_Dist > e2->m_Dist;
}

void TrackerInputDevice::trackBlobIDs(BlobVectorPtr pNewBlobs, long long time, 
        bool bTouch)
{
    TouchStatusMap * pEvents;
    string sConfigPath;
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
    float MaxDist = m_TrackerConfig.getFloatParam(sConfigPath+"similarity/@value");
    float MaxDistSquared = MaxDist*MaxDist;
    priority_queue<BlobDistEntryPtr> distHeap;
    for (BlobVector::iterator it = pNewBlobs->begin(); it != pNewBlobs->end(); ++it) {
        BlobPtr pNewBlob = *it;
        for(BlobVector::iterator it2 = oldBlobs.begin(); it2 != oldBlobs.end(); ++it2) { 
            BlobPtr pOldBlob = *it2;
            float distSquared = glm::distance2(pNewBlob->getCenter(),
                    pOldBlob->getEstimatedNextCenter());
            if (distSquared <= MaxDistSquared) {
                BlobDistEntryPtr pEntry = BlobDistEntryPtr(
                        new BlobDistEntry(distSquared, pNewBlob, pOldBlob));
                distHeap.push(pEntry);
            }
        }
    }
    // Match up the closest blobs.
    set<BlobPtr> matchedNewBlobs;
    set<BlobPtr> matchedOldBlobs;
    int numMatchedBlobs = 0;
    while (!distHeap.empty()) {
        BlobDistEntryPtr pEntry = distHeap.top();
        distHeap.pop();
        if (matchedNewBlobs.find(pEntry->m_pNewBlob) == matchedNewBlobs.end() &&
            matchedOldBlobs.find(pEntry->m_pOldBlob) == matchedOldBlobs.end())
        {
            // Found a pair of matched blobs.
            numMatchedBlobs++;
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

TrackerCalibrator* TrackerInputDevice::startCalibration()
{
    AVG_ASSERT(!m_pCalibrator);
    m_pOldTransformer = m_TrackerConfig.getTransform();
    m_OldDisplayROI = m_DisplayROI;
    m_DisplayROI = FRect(glm::vec2(0,0), glm::vec2(m_ActiveDisplaySize));
    m_TrackerConfig.setTransform(DeDistortPtr(new DeDistort(
            glm::vec2(m_pBitmaps[0]->getSize()), glm::vec2(m_ActiveDisplaySize))));
    setConfig();
    m_pCalibrator = new TrackerCalibrator(m_pBitmaps[0]->getSize(),
            m_ActiveDisplaySize);
    return m_pCalibrator;
}

void TrackerInputDevice::endCalibration()
{
    AVG_ASSERT(m_pCalibrator);
    m_TrackerConfig.setTransform(m_pCalibrator->makeTransformer());
    m_DisplayROI = m_OldDisplayROI;
    FRect area = m_TrackerConfig.getTransform()->getActiveBlobArea(m_DisplayROI);
    if (area.size().x*area.size().y > 1024*1024*8) {
        AVG_LOG_WARNING("Ignoring calibration - resulting area would be " << area);
        m_TrackerConfig.setTransform(m_pOldTransformer);
    }
    setConfig();
    delete m_pCalibrator;
    m_pCalibrator = 0;
    m_pOldTransformer = DeDistortPtr();
}

void TrackerInputDevice::abortCalibration()
{
    AVG_ASSERT(m_pCalibrator);
    m_TrackerConfig.setTransform(m_pOldTransformer);
    setConfig();
    m_pOldTransformer = DeDistortPtr();
    delete m_pCalibrator;
    m_pCalibrator = 0;
}

vector<EventPtr> TrackerInputDevice::pollEvents()
{
    lock_guard lock(*m_pMutex);
    vector<EventPtr> pTouchEvents;
    vector<EventPtr> pTrackEvents;
    pollEventType(pTouchEvents, m_TouchEvents, CursorEvent::TOUCH);
    pollEventType(pTrackEvents, m_TrackEvents, CursorEvent::TRACK);
    copyRelatedInfo(pTouchEvents, pTrackEvents);
    if (m_bFindFingertips) {
        findFingertips(pTouchEvents);
    }
    pTouchEvents.insert(pTouchEvents.end(), 
            pTrackEvents.begin(), pTrackEvents.end());
    return pTouchEvents;
}

void TrackerInputDevice::pollEventType(vector<EventPtr>& res, TouchStatusMap& Events,
        CursorEvent::Source source) 
{
    EventPtr pEvent;
    for (TouchStatusMap::iterator it = Events.begin(); it!= Events.end();) {
        TrackerTouchStatusPtr pTouchStatus = (*it).second;
        pEvent = pTouchStatus->pollEvent();
        if (pEvent) {
            res.push_back(pEvent);
            if (pEvent->getType() == Event::CURSOR_UP) {
                Events.erase(it++);
            } else {
                ++it;
            }
        } else {
            ++it;
        }
    }
}

void TrackerInputDevice::copyRelatedInfo(vector<EventPtr> pTouchEvents,
        vector<EventPtr> pTrackEvents)
{
    // Copy related blobs to related events.
    // Yuck.
    vector<EventPtr>::iterator it;
    for (it = pTouchEvents.begin(); it != pTouchEvents.end(); ++it) {
        TouchEventPtr pTouchEvent = boost::dynamic_pointer_cast<TouchEvent>(*it);
        BlobPtr pTouchBlob = pTouchEvent->getBlob();
        BlobPtr pRelatedBlob = pTouchBlob->getFirstRelated();
        if (pRelatedBlob) {
            vector<EventPtr>::iterator it2;
            TouchEventPtr pTrackEvent;
            BlobPtr pTrackBlob;
            for (it2 = pTrackEvents.begin(); 
                    pTrackBlob != pRelatedBlob && it2 != pTrackEvents.end(); 
                    ++it2) 
            {
                pTrackEvent = boost::dynamic_pointer_cast<TouchEvent>(*it2);
                pTrackBlob = pTrackEvent->getBlob();
            }
            if (pTrackBlob == pRelatedBlob) {
                pTouchEvent->addRelatedEvent(pTrackEvent);
                pTrackEvent->addRelatedEvent(pTouchEvent);
            }
        }
    }
}

void TrackerInputDevice::findFingertips(std::vector<EventPtr>& pTouchEvents)
{
    vector<EventPtr>::iterator it;
    for (it = pTouchEvents.begin(); it != pTouchEvents.end(); ++it) {
        TouchEventPtr pTouchEvent = boost::dynamic_pointer_cast<TouchEvent>(*it);
        vector<TouchEventPtr> pTrackEvents = pTouchEvent->getRelatedEvents();
        if (pTrackEvents.size() > 0) {
            float handAngle = pTouchEvent->getHandOrientation();
            float dist = glm::length(pTouchEvent->getMajorAxis())*2;
            glm::vec2 tweakVec = fromPolar(handAngle, dist);
            glm::vec2 newPos = pTouchEvent->getPos()-tweakVec;
            newPos.x = max(0.0f, min(newPos.x, float(m_ActiveDisplaySize.x)));
            newPos.y = max(0.0f, min(newPos.y, float(m_ActiveDisplaySize.y)));
            pTouchEvent->setPos(newPos);
        }
    }
}

}

