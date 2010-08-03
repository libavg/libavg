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

#include "TrackerThread.h"
#include "FilterDistortion.h"
#include "FilterWipeBorder.h"

#include "../base/Logger.h"
#include "../base/ProfilingZone.h"
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
#include "../graphics/OGLImagingContext.h"
#include "../graphics/GPUBandpassFilter.h"
#include "../graphics/GPUBlurFilter.h"

#include <iostream>
#include <stdlib.h>

using namespace std;

namespace avg {

static ProfilingZone ProfilingZoneCapture ("Capture");
static ProfilingZone ProfilingZoneMask ("Mask");
static ProfilingZone ProfilingZoneTracker ("Tracker");
static ProfilingZone ProfilingZoneHistory ("History");
static ProfilingZone ProfilingZoneDistort ("Distort");
static ProfilingZone ProfilingZoneHistogram ("Histogram");
static ProfilingZone ProfilingZoneDownscale ("Downscale");
static ProfilingZone ProfilingZoneBandpass ("Bandpass");
static ProfilingZone ProfilingZoneComps("ConnectedComps");
static ProfilingZone ProfilingZoneUpdate("Update");
static ProfilingZone ProfilingZoneDraw("Draw");

TrackerThread::TrackerThread(IntRect ROI, 
        CameraPtr pCamera,
        BitmapPtr ppBitmaps[NUM_TRACKER_IMAGES],
        MutexPtr pMutex,
        CQueue& CmdQ,
        IBlobTarget *target,
        bool bSubtractHistory,
        TrackerConfig &Config)
    : WorkerThread<TrackerThread>("Tracker", CmdQ),
      m_TouchThreshold(0),
      m_TrackThreshold(0),
      m_HistoryDelay(-1),
      m_StartTime(0),
      m_pMutex(pMutex),
      m_pCamera(pCamera),
      m_pTarget(target),
      m_pTrafo(new DeDistort()),
      m_bCreateDebugImages(false),
      m_bCreateFingerImage(false),
      m_NumFrames(0),
      m_NumCamFramesDiscarded(0),
      m_pImagingContext(0)
{
    m_bTrackBrighter = Config.getBoolParam("/tracker/brighterregions/@value");
    if (bSubtractHistory) {
        m_pHistoryPreProcessor = HistoryPreProcessorPtr(
                new HistoryPreProcessor(ppBitmaps[1]->getSize(), 1, 
                m_bTrackBrighter));
    }
    m_Prescale = Config.getIntParam("/tracker/prescale/@value");
    setBitmaps(ROI, ppBitmaps);

    DeDistortPtr pDeDistort = Config.getTransform();
    m_pDistorter = FilterDistortionPtr(new FilterDistortion(
                m_pBitmaps[TRACKER_IMG_CAMERA]->getSize()/m_Prescale, pDeDistort));

    m_pConfig = TrackerConfigPtr(new TrackerConfig(Config));
}

TrackerThread::~TrackerThread()
{
}

bool TrackerThread::init()
{
    try {
        m_pImagingContext = new OGLImagingContext();
        createBandpassFilter();
        AVG_TRACE(Logger::CONFIG, "Using fragment shaders for imaging operations.");
    } catch (Exception& e) {
        AVG_TRACE(Logger::WARNING, e.GetStr());
        AVG_TRACE(Logger::CONFIG, "Using CPU for imaging operations (slow and inaccurate).");
        m_pImagingContext = 0;
        m_pBandpassFilter = FilterPtr(new FilterFastBandpass());
    }
    try {
        m_StartTime = TimeSource::get()->getCurrentMillisecs(); 
        m_HistoryDelay = m_pConfig->getIntParam("/tracker/historydelay/@value");
    } catch (Exception& e) {
        AVG_TRACE(Logger::WARNING, e.GetStr());
    }
    
    
// Done in TrackerEventSource::ctor to work around Leopard/libdc1394 threading issue.
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
        ScopeTimer Timer(ProfilingZoneCapture);
        pCamBmp = m_pCamera->getImage(true);
        BitmapPtr pTempBmp1;
        while (pTempBmp1 = m_pCamera->getImage(false)) {
            m_NumCamFramesDiscarded++;
            m_NumFrames++;
            pCamBmp = pTempBmp1;
        }
    }
    long long time = TimeSource::get()->getCurrentMillisecs(); 
    if (pCamBmp) {
        m_NumFrames++;
        ScopeTimer Timer(ProfilingZoneTracker);
        if (m_pCameraMaskBmp) {
            ScopeTimer Timer(ProfilingZoneMask);
            FilterMask(m_pCameraMaskBmp).applyInPlace(pCamBmp);
        }
        if (m_bCreateDebugImages) {
            boost::mutex::scoped_lock Lock(*m_pMutex);
            *(m_pBitmaps[TRACKER_IMG_CAMERA]) = *pCamBmp;
            ScopeTimer Timer(ProfilingZoneHistogram);
            drawHistogram(m_pBitmaps[TRACKER_IMG_HISTOGRAM], pCamBmp);
        }
        {
            if (m_Prescale != 1) {
                ScopeTimer Timer(ProfilingZoneDownscale);
                FilterFastDownscale(m_Prescale).applyInPlace(pCamBmp);
            }
        }
        BitmapPtr pDistortedBmp;
        {
            ScopeTimer Timer(ProfilingZoneDistort);
            pDistortedBmp = m_pDistorter->apply(pCamBmp);
        }
        BitmapPtr pCroppedBmp(new Bitmap(*pDistortedBmp, m_ROI));
        if (m_bCreateDebugImages) {
            boost::mutex::scoped_lock Lock(*m_pMutex);
            m_pBitmaps[TRACKER_IMG_DISTORTED]->copyPixels(*pCroppedBmp);
        }
        if (m_pHistoryPreProcessor) {
            ScopeTimer Timer(ProfilingZoneHistory);
            m_pHistoryPreProcessor->applyInPlace(pCroppedBmp);
        }
        if (m_bCreateDebugImages) {
            boost::mutex::scoped_lock Lock(*m_pMutex);
            m_pBitmaps[TRACKER_IMG_NOHISTORY]->copyPixels(*pCroppedBmp);
            FilterNormalize(2).applyInPlace(m_pBitmaps[TRACKER_IMG_NOHISTORY]);
        }
        {
            BitmapPtr pBmpBandpass;
            if (m_TouchThreshold != 0) {
                {
                    ScopeTimer Timer(ProfilingZoneBandpass);
                    pBmpBandpass = m_pBandpassFilter->apply(pCroppedBmp);
                }
                if (m_bCreateDebugImages) {
                    boost::mutex::scoped_lock Lock(*m_pMutex);
                    *(m_pBitmaps[TRACKER_IMG_HIGHPASS]) = *pBmpBandpass;
                }
            }
            calcBlobs(pCroppedBmp, pBmpBandpass, time);
        }
    }
    return true;
}

void TrackerThread::deinit()
{
    m_pCamera = CameraPtr();
    AVG_TRACE(Logger::PROFILE, "Total camera frames: " << m_NumFrames);
    AVG_TRACE(Logger::PROFILE, "Camera frames discarded: " << m_NumCamFramesDiscarded);
    if (m_pBandpassFilter) {
        m_pBandpassFilter.reset();
    }
    if (m_pImagingContext) {
        delete m_pImagingContext;
    }
}

void TrackerThread::setConfig(TrackerConfig Config, IntRect ROI, 
        BitmapPtr ppBitmaps[NUM_TRACKER_IMAGES])
{
    boost::mutex::scoped_lock Lock(*m_pMutex);
    try {
        m_TouchThreshold = Config.getIntParam("/tracker/touch/threshold/@value");
    } catch (Exception&) {
        m_TouchThreshold = 0;
    }
    m_bTrackBrighter = Config.getBoolParam("/tracker/brighterregions/@value");
    try {
        m_TrackThreshold = Config.getIntParam("/tracker/track/threshold/@value");
    } catch (Exception&) {
        m_TrackThreshold = 0;
    }
    m_Prescale = Config.getIntParam("/tracker/prescale/@value");
    if(m_pHistoryPreProcessor) {
        m_pHistoryPreProcessor->setInterval(Config.getIntParam
                ("/tracker/historyupdateinterval/@value"));
    }
    DeDistortPtr pDeDistort = Config.getTransform();
    if (!(*m_pTrafo == *pDeDistort)) {
        m_pDistorter = FilterDistortionPtr(new FilterDistortion(
                m_pBitmaps[TRACKER_IMG_CAMERA]->getSize()/m_Prescale, pDeDistort));
        *m_pTrafo = *pDeDistort;
    }
    int Brightness = Config.getIntParam("/camera/brightness/@value");
    int Exposure = Config.getIntParam("/camera/exposure/@value");
    int Gamma = Config.getIntParam("/camera/gamma/@value");
    int Gain = Config.getIntParam("/camera/gain/@value");
    int Shutter = Config.getIntParam("/camera/shutter/@value");
    int StrobeDuration = Config.getIntParam("/camera/strobeduration/@value");
    string sCameraMaskFName = Config.getParam("/tracker/mask/@value");
    bool bNewCameraMask = ((m_pCameraMaskBmp == BitmapPtr() && sCameraMaskFName != "") || 
            m_pConfig->getParam("/tracker/mask/@value") != sCameraMaskFName);
    if (int(m_pCamera->getFeature(CAM_FEATURE_BRIGHTNESS)) != Brightness ||
             int(m_pCamera->getFeature(CAM_FEATURE_GAMMA)) != Gamma ||
             int(m_pCamera->getFeature(CAM_FEATURE_EXPOSURE)) != Exposure ||
             int(m_pCamera->getFeature(CAM_FEATURE_GAIN)) != Gain ||
             int(m_pCamera->getFeature(CAM_FEATURE_SHUTTER)) != Shutter ||
             int(m_pCamera->getFeature(CAM_FEATURE_STROBE_DURATION)) != StrobeDuration ||
             bNewCameraMask)
    {
        m_pHistoryPreProcessor->reset();
    }

    m_pCamera->setFeature(CAM_FEATURE_BRIGHTNESS, Brightness);
    m_pCamera->setFeature(CAM_FEATURE_GAMMA, Gamma);
    m_pCamera->setFeature(CAM_FEATURE_EXPOSURE, Exposure);
    m_pCamera->setFeature(CAM_FEATURE_GAIN, Gain);
    m_pCamera->setFeature(CAM_FEATURE_SHUTTER, Shutter);
    m_pCamera->setFeature(CAM_FEATURE_STROBE_DURATION, StrobeDuration, true);

    if (bNewCameraMask) {
        if (sCameraMaskFName == "") {
            m_pCameraMaskBmp = BitmapPtr();
        } else {
            BitmapPtr pRGBXCameraMaskBmp = BitmapPtr(new Bitmap(sCameraMaskFName));
            m_pCameraMaskBmp = BitmapPtr(
                    new Bitmap(pRGBXCameraMaskBmp->getSize(), I8));
            m_pCameraMaskBmp->copyPixels(*pRGBXCameraMaskBmp);        
        }
    }
    m_pConfig = TrackerConfigPtr(new TrackerConfig(Config));
        
    setBitmaps(ROI, ppBitmaps);
    createBandpassFilter();
}

void TrackerThread::setDebugImages(bool bImg, bool bFinger)
{
    m_bCreateDebugImages = bImg;
    m_bCreateFingerImage = bFinger;
}

void TrackerThread::setBitmaps(IntRect ROI, BitmapPtr ppBitmaps[NUM_TRACKER_IMAGES])
{
    m_ROI = ROI;
    for (int i=0; i<NUM_TRACKER_IMAGES; i++) {
        m_pBitmaps[i] = ppBitmaps[i];
    }
    if (m_pHistoryPreProcessor) {
        m_pHistoryPreProcessor = HistoryPreProcessorPtr(
                new HistoryPreProcessor(ROI.size(), 
                        m_pHistoryPreProcessor->getInterval(), m_bTrackBrighter));
    }
    if (m_pBandpassFilter) {
        createBandpassFilter();
    }
}

void TrackerThread::createBandpassFilter()
{
    if (m_TouchThreshold != 0) {
        double bandpassMin = m_pConfig->getDoubleParam("/tracker/touch/bandpass/@min");
        double bandpassMax = m_pConfig->getDoubleParam("/tracker/touch/bandpass/@max");
        double bandpassPostMult = 
            m_pConfig->getDoubleParam("/tracker/touch/bandpasspostmult/@value");
        if (m_pImagingContext) {
            m_pBandpassFilter = FilterPtr(new GPUBandpassFilter(m_ROI.size(), I8,
                        bandpassMin, bandpassMax, bandpassPostMult, m_bTrackBrighter));
        }
    }
}

void TrackerThread::resetHistory()
{
    if(m_pHistoryPreProcessor)
        m_pHistoryPreProcessor->reset();
}
        
void TrackerThread::drawHistogram(BitmapPtr pDestBmp, BitmapPtr pSrcBmp)
{
    HistogramPtr pHist = pSrcBmp->getHistogram(4);
    AVG_ASSERT(pDestBmp->getPixelFormat() == I8);
    // Normalize Histogram to 0..255
    int Max1 = 0;
    int Max2 = 0;
    for (int i=0; i<256; ++i) {
        if ((*pHist)[i] > Max1) {
            Max2 = Max1;
            Max1 = (*pHist)[i];
        } else if ((*pHist)[i] > Max2) {
            Max2 = (*pHist)[i];
        }
    }
    if (Max2== 0) {
        Max2= 1;
    }
    for (int i=0; i<256; ++i) {
        (*pHist)[i] = int((*pHist)[i]*256.0/Max2)+1;
    }
    
    FilterFill<Pixel8>(0).applyInPlace(pDestBmp);
    int Stride = pDestBmp->getStride();
    int EndRow = 256;
    if (pDestBmp->getSize().y < 256) {
        EndRow = pDestBmp->getSize().y;
    }
    int Width = pDestBmp->getSize().x;
    for (int i=0; i<EndRow; ++i) {
        int EndCol =(*pHist)[i];
        if (EndCol > Width) { 
            EndCol = Width;
        }
        unsigned char * pDest = pDestBmp->getPixels()+Stride*i;
        memset(pDest, 255, EndCol);
    }
}

inline bool isInbetween(double x, double min, double max)
{
    return x>=min && x<=max;
}

bool TrackerThread::isRelevant(BlobPtr pBlob, int MinArea, int MaxArea,
        double MinEccentricity, double MaxEccentricity)
{
    bool res;
    res = isInbetween(pBlob->getArea(), MinArea, MaxArea) && 
            isInbetween(pBlob->getEccentricity(), MinEccentricity, MaxEccentricity);
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
    int MinArea = m_pConfig->getIntParam(sConfigPrefix+"areabounds/@min");
    int MaxArea = m_pConfig->getIntParam(sConfigPrefix+"areabounds/@max");
    double MinEccentricity = m_pConfig->getDoubleParam(sConfigPrefix+"eccentricitybounds/@min");
    double MaxEccentricity = m_pConfig->getDoubleParam(sConfigPrefix+"eccentricitybounds/@max");
    
    BlobVectorPtr pRelevantBlobs(new BlobVector());
    for(BlobVector::iterator it = pBlobs->begin(); it!=pBlobs->end(); ++it) {
        if (isRelevant(*it, MinArea, MaxArea, MinEccentricity, MaxEccentricity)) {
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
    ScopeTimer Timer(ProfilingZoneDraw);
    string sConfigPrefix;
    if (bTouch) {
        sConfigPrefix = "/tracker/touch/";
    } else {
        sConfigPrefix = "/tracker/track/";
    }
    int MinArea = m_pConfig->getIntParam(sConfigPrefix+"areabounds/@min");
    int MaxArea = m_pConfig->getIntParam(sConfigPrefix+"areabounds/@max");
    double MinEccentricity = m_pConfig->getDoubleParam(sConfigPrefix+"eccentricitybounds/@min");
    double MaxEccentricity = m_pConfig->getDoubleParam(sConfigPrefix+"eccentricitybounds/@max");
    
    // Get max. pixel value in Bitmap
    int Max = 0;
    HistogramPtr pHist = pSrcBmp->getHistogram(4);
    int i;
    for(i=255; i>=0; i--) {
        if ((*pHist)[i] != 0) {
            Max = i;
            i = 0;
        }
    }
    
    for(BlobVector::iterator it2 = pBlobs->begin();it2!=pBlobs->end();++it2) {
        if (isRelevant(*it2, MinArea, MaxArea, MinEccentricity, MaxEccentricity)) {
            if (bTouch) {
                (*it2)->render(pSrcBmp, pDestBmp, 
                        Pixel32(0xFF, 0xFF, 0xFF, 0xFF), Offset, Max, bTouch, true,  
                        Pixel32(0x00, 0x00, 0xFF, 0xFF));
            } else {
                (*it2)->render(pSrcBmp, pDestBmp, 
                        Pixel32(0xFF, 0xFF, 0x00, 0x80), Offset, Max, bTouch, true, 
                        Pixel32(0x00, 0x00, 0xFF, 0xFF));
            }
        } else {
            if (bTouch) {
                (*it2)->render(pSrcBmp, pDestBmp, 
                        Pixel32(0xFF, 0x00, 0x00, 0xFF), Offset, Max, bTouch, false);
            } else {
                (*it2)->render(pSrcBmp, pDestBmp, 
                        Pixel32(0x80, 0x80, 0x00, 0x80), Offset, Max, bTouch, false);
            }
        }
    }
}

void TrackerThread::calcContours(BlobVectorPtr pBlobs)
{
    ScopeTimer Timer(ProfilingZoneDraw);
    string sConfigPrefix;
    sConfigPrefix = "/tracker/track/";
    int MinArea = m_pConfig->getIntParam(sConfigPrefix+"areabounds/@min");
    int MaxArea = m_pConfig->getIntParam(sConfigPrefix+"areabounds/@max");
    double MinEccentricity = m_pConfig->getDoubleParam(sConfigPrefix+"eccentricitybounds/@min");
    double MaxEccentricity = m_pConfig->getDoubleParam(sConfigPrefix+"eccentricitybounds/@max");
    
    int ContourPrecision = m_pConfig->getIntParam("/tracker/contourprecision/@value");
    if (ContourPrecision != 0) {
        for(BlobVector::iterator it = pBlobs->begin(); it!=pBlobs->end(); ++it) {
            if (isRelevant(*it, MinArea, MaxArea, MinEccentricity, MaxEccentricity)) {
                (*it)->calcContour(ContourPrecision);
            }
        }
    }
}

void TrackerThread::correlateHands(BlobVectorPtr pTrackBlobs, 
        BlobVectorPtr pTouchBlobs)
{
   if (!pTrackBlobs || !pTouchBlobs) {
       return;
   }
    for(BlobVector::iterator it1=pTouchBlobs->begin(); 
            it1 != pTouchBlobs->end(); ++it1) 
    {
        BlobPtr pTouchBlob = *it1;
        IntPoint TouchCenter = (IntPoint)(pTouchBlob->getCenter());
        for(BlobVector::iterator it2=pTrackBlobs->begin(); 
                it2!=pTrackBlobs->end(); ++it2) 
        {
            BlobPtr pTrackBlob = *it2;
            if (pTrackBlob->contains(TouchCenter)) {
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
        ScopeTimer Timer(ProfilingZoneComps);
        boost::mutex::scoped_lock Lock(*m_pMutex);
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
            ScopeTimer Timer(ProfilingZoneUpdate);
            m_pTarget->update(pTrackComps, pTouchComps, time);
        }
    }
    
}
        
}
