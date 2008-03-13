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

#include "TrackerThread.h"
#include "FilterDistortion.h"

#include "../base/Logger.h"
#include "../base/ProfilingZone.h"
#include "../base/ScopeTimer.h"

#include "../graphics/Filter.h"
#include "../graphics/Filterfill.h"
#include "../graphics/FilterHighpass.h"
#include "../graphics/FilterFastBandpass.h"
#include "../graphics/FilterFastDownscale.h"
#include "../graphics/FilterBlur.h"
#include "../graphics/FilterGauss.h"
#include "../graphics/FilterMask.h"

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
        CmdQueue& CmdQ,
        IBlobTarget *target,
        bool bSubtractHistory,
        TrackerConfig &Config)
    : WorkerThread<TrackerThread>("Tracker", CmdQ),
      m_TouchThreshold(0),
      m_TrackThreshold(0),
      m_pMutex(pMutex),
      m_pCamera(pCamera),
      m_pTarget(target),
      m_pTrafo(new DeDistort()),
      m_bCreateDebugImages(false),
      m_bCreateFingerImage(false)
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
}

TrackerThread::~TrackerThread()
{
}

bool TrackerThread::init()
{
// Done in TrackerEventSource::ctor to work around Leopard/libdc1394 threading issue.
//    m_pCamera->open();
    return true;
}

bool TrackerThread::work()
{
    BitmapPtr pCamBmp;
    {
        ScopeTimer Timer(ProfilingZoneCapture);
        pCamBmp = m_pCamera->getImage(true);
        BitmapPtr pTempBmp1;
        while (pTempBmp1 = m_pCamera->getImage(false)) {
            pCamBmp = pTempBmp1;
        }
    }
    if (pCamBmp) {
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
            ScopeTimer Timer(ProfilingZoneDownscale);
            if (m_Prescale != 1) {
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
        }
        {
            BitmapPtr pBmpBandpass;
            if (m_TouchThreshold != 0) {
                {
                    ScopeTimer Timer(ProfilingZoneBandpass);
                    pBmpBandpass = FilterFastBandpass().apply(pCroppedBmp);
                }
                if (m_bCreateDebugImages) {
                    boost::mutex::scoped_lock Lock(*m_pMutex);
                    *(m_pBitmaps[TRACKER_IMG_HIGHPASS]) = *pBmpBandpass;
                }
            }
            calcBlobs(pCroppedBmp, pBmpBandpass);
        }
    }
    return true;
}

void TrackerThread::deinit()
{
    m_pCamera->close();
}

void TrackerThread::setConfig(TrackerConfig Config, IntRect ROI, 
        BitmapPtr ppBitmaps[NUM_TRACKER_IMAGES])
{
    boost::mutex::scoped_lock Lock(*m_pMutex);
    try {
        m_TouchThreshold = Config.getIntParam("/tracker/touch/threshold/@value");
    } catch (Exception& e) {
        m_TouchThreshold = 0;
    }
    m_bTrackBrighter = Config.getBoolParam("/tracker/brighterregions/@value");
    try {
        m_TrackThreshold = Config.getIntParam("/tracker/track/threshold/@value");
    } catch (Exception& e) {
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
    string sCameraMaskFName = Config.getParam("/tracker/mask/@value");
    if (int(m_pCamera->getFeature(CAM_FEATURE_BRIGHTNESS)) != Brightness ||
            int(m_pCamera->getFeature(CAM_FEATURE_EXPOSURE)) != Exposure ||
             int(m_pCamera->getFeature(CAM_FEATURE_GAMMA)) != Gamma ||
             int(m_pCamera->getFeature(CAM_FEATURE_GAIN)) != Gain ||
             int(m_pCamera->getFeature(CAM_FEATURE_SHUTTER)) != Shutter ||
             m_sCameraMaskFName != sCameraMaskFName)
    {
        m_pHistoryPreProcessor->reset();
    }

    m_pCamera->setFeature(CAM_FEATURE_BRIGHTNESS, Brightness);
    m_pCamera->setFeature(CAM_FEATURE_EXPOSURE, Exposure);
    m_pCamera->setFeature(CAM_FEATURE_GAMMA, Gamma);
    m_pCamera->setFeature(CAM_FEATURE_GAIN, Gain);
    m_pCamera->setFeature(CAM_FEATURE_SHUTTER, Shutter);

    if (m_sCameraMaskFName != sCameraMaskFName) {
        m_sCameraMaskFName = sCameraMaskFName;
        if (m_sCameraMaskFName == "") {
            m_pCameraMaskBmp = BitmapPtr();
        } else {
            BitmapPtr pRGBXCameraMaskBmp = BitmapPtr(new Bitmap(m_sCameraMaskFName));
            m_pCameraMaskBmp = BitmapPtr(
                new Bitmap(pRGBXCameraMaskBmp->getSize(), I8));
            m_pCameraMaskBmp->copyPixels(*pRGBXCameraMaskBmp);        
        }
    }

    setBitmaps(ROI, ppBitmaps);
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
}

void TrackerThread::resetHistory()
{
    if(m_pHistoryPreProcessor)
        m_pHistoryPreProcessor->reset();
}
        
void TrackerThread::drawHistogram(BitmapPtr pDestBmp, BitmapPtr pSrcBmp)
{
    HistogramPtr pHist = pSrcBmp->getHistogram(3);
    assert(pDestBmp->getPixelFormat() == I8);
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

void TrackerThread::calcBlobs(BitmapPtr pTrackBmp, BitmapPtr pTouchBmp) {
    BlobVectorPtr pTrackComps;
    BlobVectorPtr pTouchComps;
    {
        ScopeTimer Timer(ProfilingZoneComps);
        pTrackComps = connected_components(pTrackBmp, m_TrackThreshold);
        pTouchComps = connected_components(pTouchBmp, m_TouchThreshold);
    }
    //feed the IBlobTarget
    BitmapPtr pDestBmp;
    {
        boost::mutex::scoped_lock Lock(*m_pMutex);
        if (m_bCreateFingerImage) {
            Pixel32 Black(0x00, 0x00, 0x00, 0x00);
            FilterFill<Pixel32>(Black).applyInPlace(
                    m_pBitmaps[TRACKER_IMG_FINGERS]);
            pDestBmp = m_pBitmaps[TRACKER_IMG_FINGERS];
        }
        ScopeTimer Timer(ProfilingZoneUpdate);
        m_pTarget->update(pTrackComps, pTrackBmp, m_TrackThreshold,
                pTouchComps, pTouchBmp, m_TouchThreshold, pDestBmp);
    }
}
        
}
