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

#include "TrackerThread.h"
#include "FilterDistortion.h"
#include "FilterWipeBorder.h"

#include "../base/Logger.h"
#include "../base/ProfilingZoneID.h"
#include "../base/TimeSource.h"
#include "../base/ScopeTimer.h"
#include "../base/Exception.h"

#include "../graphics/Filter.h"
#include "../graphics/Filterfill.h"
#include "../graphics/FilterHighpass.h"
#include "../graphics/FilterFastBandpass.h"
#include "../graphics/FilterFastDownscale.h"
#include "../graphics/FilterNormalize.h"
#include "../graphics/FilterBlur.h"
#include "../graphics/FilterGauss.h"
#include "../graphics/FilterMask.h"
#include "../graphics/GLContext.h"
#include "../graphics/GPUBandpassFilter.h"
#include "../graphics/GPUBlurFilter.h"
#include "../graphics/BitmapLoader.h"

#include <iostream>
#include <stdlib.h>

using namespace std;

namespace avg {

static ProfilingZoneID ProfilingZoneCapture("Capture");
static ProfilingZoneID ProfilingZoneMask("Mask");
static ProfilingZoneID ProfilingZoneTracker("Tracker");
static ProfilingZoneID ProfilingZoneHistory("History");
static ProfilingZoneID ProfilingZoneDistort("Distort");
static ProfilingZoneID ProfilingZoneHistogram("Histogram");
static ProfilingZoneID ProfilingZoneDownscale("Downscale");
static ProfilingZoneID ProfilingZoneBandpass("Bandpass");
static ProfilingZoneID ProfilingZoneComps("ConnectedComps");
static ProfilingZoneID ProfilingZoneUpdate("Update");
static ProfilingZoneID ProfilingZoneDraw("Draw");

TrackerThread::TrackerThread(IntRect roi, CameraPtr pCamera,
        BitmapPtr ppBitmaps[NUM_TRACKER_IMAGES], MutexPtr pMutex, CQueue& cmdQ,
        IBlobTarget *pTarget, bool bSubtractHistory, TrackerConfig& config)
    : WorkerThread<TrackerThread>("Tracker", cmdQ),
      m_TouchThreshold(0),
      m_TrackThreshold(0),
      m_HistoryDelay(-1),
      m_StartTime(0),
      m_pMutex(pMutex),
      m_pCamera(pCamera),
      m_pTarget(pTarget),
      m_pTrafo(new DeDistort()),
      m_bCreateDebugImages(false),
      m_bCreateFingerImage(false),
      m_NumFrames(0),
      m_NumCamFramesDiscarded(0),
      m_pImagingContext(0)
{
    m_bTrackBrighter = config.getBoolParam("/tracker/brighterregions/@value");
    if (bSubtractHistory) {
        m_pHistoryPreProcessor = HistoryPreProcessorPtr(
                new HistoryPreProcessor(ppBitmaps[1]->getSize(), 1, 
                m_bTrackBrighter));
    }
    m_Prescale = config.getIntParam("/tracker/prescale/@value");
    setBitmaps(roi, ppBitmaps);

    DeDistortPtr pDeDistort = config.getTransform();
    m_pDistorter = FilterDistortionPtr(new FilterDistortion(
                m_pBitmaps[TRACKER_IMG_CAMERA]->getSize()/m_Prescale, pDeDistort));

    m_pConfig = TrackerConfigPtr(new TrackerConfig(config));
    m_pCamera->startCapture();
}

TrackerThread::~TrackerThread()
{
}

bool TrackerThread::init()
{
    try {
        m_pImagingContext = GLContext::create(
            GLConfig(false, false, true, 1, GLConfig::AUTO, false));
        createBandpassFilter();
        AVG_TRACE(Logger::category::CONFIG, Logger::severity::INFO,
                "Using fragment shaders for imaging operations.");
    } catch (Exception& e) {
        AVG_LOG_WARNING(e.getStr());
        AVG_TRACE(Logger::category::CONFIG, Logger::severity::WARNING,
                "Using CPU for imaging operations (slow and inaccurate).");
        m_pImagingContext = 0;
        m_pBandpassFilter = FilterPtr(new FilterFastBandpass());
    }
    try {
        m_StartTime = TimeSource::get()->getCurrentMillisecs(); 
        m_HistoryDelay = m_pConfig->getIntParam("/tracker/historydelay/@value");
    } catch (Exception& e) {
        AVG_LOG_WARNING(e.getStr());
    }
    
    // Done in TrackerInputDevice::ctor to work around Leopard/libdc1394 threading issue.
    //    m_pCamera->open();
    return true;
}

bool TrackerThread::work()
{
    if ((m_HistoryDelay + m_StartTime) < TimeSource::get()->getCurrentMillisecs() 
            && m_HistoryDelay != -1) 
    {   
        resetHistory();
        m_HistoryDelay = -1;
    }
    
    BitmapPtr pCamBmp;
    {
        ScopeTimer timer(ProfilingZoneCapture);
        pCamBmp = m_pCamera->getImage(true);
        BitmapPtr pTempBmp1;
        while ((pTempBmp1 = m_pCamera->getImage(false))) {
            m_NumCamFramesDiscarded++;
            m_NumFrames++;
            pCamBmp = pTempBmp1;
        }
    }
    long long time = TimeSource::get()->getCurrentMillisecs(); 
    if (pCamBmp) {
        m_NumFrames++;
        ScopeTimer timer(ProfilingZoneTracker);
        if (m_pCameraMaskBmp) {
            ScopeTimer timer(ProfilingZoneMask);
            FilterMask(m_pCameraMaskBmp).applyInPlace(pCamBmp);
        }
        if (m_bCreateDebugImages) {
            lock_guard lock(*m_pMutex);
            *(m_pBitmaps[TRACKER_IMG_CAMERA]) = *pCamBmp;
            ScopeTimer timer(ProfilingZoneHistogram);
            drawHistogram(m_pBitmaps[TRACKER_IMG_HISTOGRAM], pCamBmp);
        }
        {
            if (m_Prescale != 1) {
                ScopeTimer timer(ProfilingZoneDownscale);
                FilterFastDownscale(m_Prescale).applyInPlace(pCamBmp);
            }
        }
        BitmapPtr pDistortedBmp;
        {
            ScopeTimer timer(ProfilingZoneDistort);
            pDistortedBmp = m_pDistorter->apply(pCamBmp);
        }
        BitmapPtr pCroppedBmp(new Bitmap(*pDistortedBmp, m_ROI));
        if (m_bCreateDebugImages) {
            lock_guard lock(*m_pMutex);
            m_pBitmaps[TRACKER_IMG_DISTORTED]->copyPixels(*pCroppedBmp);
        }
        if (m_pHistoryPreProcessor) {
            ScopeTimer timer(ProfilingZoneHistory);
            m_pHistoryPreProcessor->applyInPlace(pCroppedBmp);
        }
        if (m_bCreateDebugImages) {
            lock_guard lock(*m_pMutex);
            m_pBitmaps[TRACKER_IMG_NOHISTORY]->copyPixels(*pCroppedBmp);
            FilterNormalize(2).applyInPlace(m_pBitmaps[TRACKER_IMG_NOHISTORY]);
        }
        {
            BitmapPtr pBmpBandpass;
            if (m_TouchThreshold != 0) {
                {
                    ScopeTimer timer(ProfilingZoneBandpass);
                    pBmpBandpass = m_pBandpassFilter->apply(pCroppedBmp);
                }
                if (m_bCreateDebugImages) {
                    lock_guard lock(*m_pMutex);
                    *(m_pBitmaps[TRACKER_IMG_HIGHPASS]) = *pBmpBandpass;
                }
            }
            calcBlobs(pCroppedBmp, pBmpBandpass, time);
        }
        ThreadProfiler::get()->reset();
    }
    return true;
}

void TrackerThread::deinit()
{
    m_pCamera = CameraPtr();
    AVG_TRACE(Logger::category::PROFILE, Logger::severity::INFO,
            "Total camera frames: " << m_NumFrames);
    AVG_TRACE(Logger::category::PROFILE, Logger::severity::INFO,
            "Camera frames discarded: " << m_NumCamFramesDiscarded);
    if (m_pBandpassFilter) {
        m_pBandpassFilter.reset();
    }
    if (m_pImagingContext) {
        delete m_pImagingContext;
    }
}

void TrackerThread::setConfig(TrackerConfig config, IntRect roi, 
        BitmapPtr ppBitmaps[NUM_TRACKER_IMAGES])
{
    lock_guard lock(*m_pMutex);
    try {
        m_TouchThreshold = config.getIntParam("/tracker/touch/threshold/@value");
    } catch (Exception&) {
        m_TouchThreshold = 0;
    }
    m_bTrackBrighter = config.getBoolParam("/tracker/brighterregions/@value");
    try {
        m_TrackThreshold = config.getIntParam("/tracker/track/threshold/@value");
    } catch (Exception&) {
        m_TrackThreshold = 0;
    }
    m_Prescale = config.getIntParam("/tracker/prescale/@value");
    if(m_pHistoryPreProcessor) {
        m_pHistoryPreProcessor->setInterval(config.getIntParam
                ("/tracker/historyupdateinterval/@value"));
    }
    DeDistortPtr pDeDistort = config.getTransform();
    if (!(*m_pTrafo == *pDeDistort)) {
        m_pDistorter = FilterDistortionPtr(new FilterDistortion(
                m_pBitmaps[TRACKER_IMG_CAMERA]->getSize()/m_Prescale, pDeDistort));
        *m_pTrafo = *pDeDistort;
    }
    int brightness = config.getIntParam("/camera/brightness/@value");
    int exposure = config.getIntParam("/camera/exposure/@value");
    int gamma = config.getIntParam("/camera/gamma/@value");
    int gain = config.getIntParam("/camera/gain/@value");
    int shutter = config.getIntParam("/camera/shutter/@value");
    int strobeDuration = config.getIntParam("/camera/strobeduration/@value");
    string sCameraMaskFName = config.getParam("/tracker/mask/@value");
    bool bNewCameraMask = ((m_pCameraMaskBmp == BitmapPtr() && sCameraMaskFName != "") || 
            m_pConfig->getParam("/tracker/mask/@value") != sCameraMaskFName);
    if (int(m_pCamera->getFeature(CAM_FEATURE_BRIGHTNESS)) != brightness ||
             int(m_pCamera->getFeature(CAM_FEATURE_GAMMA)) != gamma ||
             int(m_pCamera->getFeature(CAM_FEATURE_EXPOSURE)) != exposure ||
             int(m_pCamera->getFeature(CAM_FEATURE_GAIN)) != gain ||
             int(m_pCamera->getFeature(CAM_FEATURE_SHUTTER)) != shutter ||
             int(m_pCamera->getFeature(CAM_FEATURE_STROBE_DURATION)) != strobeDuration ||
             bNewCameraMask)
    {
        m_pHistoryPreProcessor->reset();
    }

    m_pCamera->setFeature(CAM_FEATURE_BRIGHTNESS, brightness);
    m_pCamera->setFeature(CAM_FEATURE_GAMMA, gamma);
//    m_pCamera->setFeature(CAM_FEATURE_EXPOSURE, exposure);
    m_pCamera->setFeature(CAM_FEATURE_GAIN, gain);
    m_pCamera->setFeature(CAM_FEATURE_SHUTTER, shutter);
    m_pCamera->setFeature(CAM_FEATURE_STROBE_DURATION, strobeDuration, true);

    if (bNewCameraMask) {
        if (sCameraMaskFName == "") {
            m_pCameraMaskBmp = BitmapPtr();
        } else {
            BitmapPtr pRGBXCameraMaskBmp = loadBitmap(sCameraMaskFName, I8);
        }
    }
    m_pConfig = TrackerConfigPtr(new TrackerConfig(config));
        
    setBitmaps(roi, ppBitmaps);
    createBandpassFilter();
}

void TrackerThread::setDebugImages(bool bImg, bool bFinger)
{
    m_bCreateDebugImages = bImg;
    m_bCreateFingerImage = bFinger;
}

void TrackerThread::setBitmaps(IntRect roi, BitmapPtr ppBitmaps[NUM_TRACKER_IMAGES])
{
    m_ROI = roi;
    for (int i=0; i<NUM_TRACKER_IMAGES; i++) {
        m_pBitmaps[i] = ppBitmaps[i];
    }
    if (m_pHistoryPreProcessor) {
        m_pHistoryPreProcessor = HistoryPreProcessorPtr(
                new HistoryPreProcessor(roi.size(), 
                        m_pHistoryPreProcessor->getInterval(), m_bTrackBrighter));
    }
    if (m_pBandpassFilter) {
        createBandpassFilter();
    }
}

void TrackerThread::createBandpassFilter()
{
    if (m_TouchThreshold != 0) {
        float bandpassMin = m_pConfig->getFloatParam("/tracker/touch/bandpass/@min");
        float bandpassMax = m_pConfig->getFloatParam("/tracker/touch/bandpass/@max");
        float bandpassPostMult = 
            m_pConfig->getFloatParam("/tracker/touch/bandpasspostmult/@value");
        if (m_pImagingContext) {
            m_pBandpassFilter = FilterPtr(new GPUBandpassFilter(m_ROI.size(), I8,
                        bandpassMin, bandpassMax, bandpassPostMult, m_bTrackBrighter));
        }
    }
}

void TrackerThread::resetHistory()
{
    if (m_pHistoryPreProcessor) {
        m_pHistoryPreProcessor->reset();
    }
}
        
void TrackerThread::drawHistogram(BitmapPtr pDestBmp, BitmapPtr pSrcBmp)
{
    HistogramPtr pHist = pSrcBmp->getHistogram(4);
    AVG_ASSERT(pDestBmp->getPixelFormat() == I8);
    // Normalize Histogram to 0..255
    int max1 = 0;
    int max2 = 0;
    for (int i = 0; i < 256; ++i) {
        if ((*pHist)[i] > max1) {
            max2 = max1;
            max1 = (*pHist)[i];
        } else if ((*pHist)[i] > max2) {
            max2 = (*pHist)[i];
        }
    }
    if (max2 == 0) {
        max2= 1;
    }
    for (int i = 0; i < 256; ++i) {
        (*pHist)[i] = int((*pHist)[i]*256.0/max2)+1;
    }
    
    FilterFill<Pixel8>(0).applyInPlace(pDestBmp);
    int stride = pDestBmp->getStride();
    int endRow = 256;
    if (pDestBmp->getSize().y < 256) {
        endRow = pDestBmp->getSize().y;
    }
    int width = pDestBmp->getSize().x;
    for (int i = 0; i < endRow; ++i) {
        int endCol =(*pHist)[i];
        if (endCol > width) { 
            endCol = width;
        }
        unsigned char * pDest = pDestBmp->getPixels()+stride*i;
        memset(pDest, 255, endCol);
    }
}

inline bool isInbetween(float x, float min, float max)
{
    return x >= min && x <= max;
}

bool TrackerThread::isRelevant(BlobPtr pBlob, int minArea, int maxArea,
        float minEccentricity, float maxEccentricity)
{
    bool res;
    res = isInbetween(pBlob->getArea(), float(minArea), float(maxArea)) && 
            isInbetween(pBlob->getEccentricity(), minEccentricity, maxEccentricity);
    return res;
}

BlobVectorPtr TrackerThread::findRelevantBlobs(BlobVectorPtr pBlobs, bool bTouch) 
{
    string sConfigPrefix;
    if (bTouch) {
        sConfigPrefix = "/tracker/touch/";
    } else {
        sConfigPrefix = "/tracker/track/";
    }
    int minArea = m_pConfig->getIntParam(sConfigPrefix+"areabounds/@min");
    int maxArea = m_pConfig->getIntParam(sConfigPrefix+"areabounds/@max");
    float minEccentricity = m_pConfig->getFloatParam(sConfigPrefix+
            "eccentricitybounds/@min");
    float maxEccentricity = m_pConfig->getFloatParam(sConfigPrefix+
            "eccentricitybounds/@max");
    
    BlobVectorPtr pRelevantBlobs(new BlobVector());
    for(BlobVector::iterator it = pBlobs->begin(); it != pBlobs->end(); ++it) {
        if (isRelevant(*it, minArea, maxArea, minEccentricity, maxEccentricity)) {
            pRelevantBlobs->push_back(*it);
        }
        if (pRelevantBlobs->size() > 50) {
            break;
        }
    }
    return pRelevantBlobs;
}

void TrackerThread::drawBlobs(BlobVectorPtr pBlobs, BitmapPtr pSrcBmp, 
        BitmapPtr pDestBmp, int Offset, bool bTouch)
{
    if (!pDestBmp) {
        return;
    }
    ScopeTimer timer(ProfilingZoneDraw);
    string sConfigPrefix;
    if (bTouch) {
        sConfigPrefix = "/tracker/touch/";
    } else {
        sConfigPrefix = "/tracker/track/";
    }
    int minArea = m_pConfig->getIntParam(sConfigPrefix+"areabounds/@min");
    int maxArea = m_pConfig->getIntParam(sConfigPrefix+"areabounds/@max");
    float minEccentricity = m_pConfig->getFloatParam(
            sConfigPrefix+"eccentricitybounds/@min");
    float maxEccentricity = m_pConfig->getFloatParam(
            sConfigPrefix+"eccentricitybounds/@max");
    
    // Get max. pixel value in Bitmap
    int max = 0;
    HistogramPtr pHist = pSrcBmp->getHistogram(4);
    int i;
    for (i = 255; i >= 0; i--) {
        if ((*pHist)[i] != 0) {
            max = i;
            i = 0;
        }
    }
    
    for (BlobVector::iterator it2 = pBlobs->begin(); it2 != pBlobs->end(); ++it2) {
        if (isRelevant(*it2, minArea, maxArea, minEccentricity, maxEccentricity)) {
            if (bTouch) {
                (*it2)->render(pSrcBmp, pDestBmp, 
                        Pixel32(0xFF, 0xFF, 0xFF, 0xFF), Offset, max, bTouch, true,  
                        Pixel32(0x00, 0x00, 0xFF, 0xFF));
            } else {
                (*it2)->render(pSrcBmp, pDestBmp, 
                        Pixel32(0xFF, 0xFF, 0x00, 0x80), Offset, max, bTouch, true, 
                        Pixel32(0x00, 0x00, 0xFF, 0xFF));
            }
        } else {
            if (bTouch) {
                (*it2)->render(pSrcBmp, pDestBmp, 
                        Pixel32(0xFF, 0x00, 0x00, 0xFF), Offset, max, bTouch, false);
            } else {
                (*it2)->render(pSrcBmp, pDestBmp, 
                        Pixel32(0x80, 0x80, 0x00, 0x80), Offset, max, bTouch, false);
            }
        }
    }
}

void TrackerThread::calcContours(BlobVectorPtr pBlobs)
{
    ScopeTimer timer(ProfilingZoneDraw);
    string sConfigPrefix;
    sConfigPrefix = "/tracker/track/";
    int minArea = m_pConfig->getIntParam(sConfigPrefix+"areabounds/@min");
    int maxArea = m_pConfig->getIntParam(sConfigPrefix+"areabounds/@max");
    float minEccentricity = m_pConfig->getFloatParam(
            sConfigPrefix+"eccentricitybounds/@min");
    float maxEccentricity = m_pConfig->getFloatParam(
            sConfigPrefix+"eccentricitybounds/@max");
    
    int ContourPrecision = m_pConfig->getIntParam("/tracker/contourprecision/@value");
    if (ContourPrecision != 0) {
        for (BlobVector::iterator it = pBlobs->begin(); it != pBlobs->end(); ++it) {
            if (isRelevant(*it, minArea, maxArea, minEccentricity, maxEccentricity)) {
                (*it)->calcContour(ContourPrecision);
            }
        }
    }
}

void TrackerThread::correlateHands(BlobVectorPtr pTrackBlobs, BlobVectorPtr pTouchBlobs)
{
   if (!pTrackBlobs || !pTouchBlobs) {
       return;
   }
    for (BlobVector::iterator it1 = pTouchBlobs->begin(); it1 != pTouchBlobs->end();
            ++it1) 
    {
        BlobPtr pTouchBlob = *it1;
        IntPoint touchCenter = (IntPoint)(pTouchBlob->getCenter());
        for (BlobVector::iterator it2 = pTrackBlobs->begin(); it2 != pTrackBlobs->end(); 
                ++it2) 
        {
            BlobPtr pTrackBlob = *it2;
            if (pTrackBlob->contains(touchCenter)) {
                pTouchBlob->addRelated(pTrackBlob);
                pTrackBlob->addRelated(pTouchBlob);
                break;
            }
        }
    }
}

void TrackerThread::calcBlobs(BitmapPtr pTrackBmp, BitmapPtr pTouchBmp, long long time) 
{
    BlobVectorPtr pTrackComps;
    BlobVectorPtr pTouchComps;
    {
        ScopeTimer timer(ProfilingZoneComps);
        lock_guard lock(*m_pMutex);
        BitmapPtr pDestBmp;
        if (m_bCreateFingerImage) {
            Pixel32 Black(0x00, 0x00, 0x00, 0x00);
            FilterFill<Pixel32>(Black).applyInPlace(
                    m_pBitmaps[TRACKER_IMG_FINGERS]);
            pDestBmp = m_pBitmaps[TRACKER_IMG_FINGERS];
        }
        {
            if (m_TrackThreshold != 0) {
                pTrackComps = findConnectedComponents(pTrackBmp, m_TrackThreshold);
                calcContours(pTrackComps);
                drawBlobs(pTrackComps, pTrackBmp, pDestBmp, m_TrackThreshold, false);
                pTrackComps = findRelevantBlobs(pTrackComps, false);
            }
            if (m_TouchThreshold != 0) {
                pTouchComps = findConnectedComponents(pTouchBmp, m_TouchThreshold);
                pTouchComps = findRelevantBlobs(pTouchComps, true);
                correlateHands(pTrackComps, pTouchComps);
                drawBlobs(pTouchComps, pTouchBmp, pDestBmp, m_TouchThreshold, true);
            }
        }
        // Send the blobs to the BlobTarget.
        {
            ScopeTimer timer(ProfilingZoneUpdate);
            m_pTarget->update(pTrackComps, pTouchComps, time);
        }
    }
    
}
        
}
