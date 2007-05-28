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
#include "../graphics/FilterBlur.h"
#include "../graphics/FilterGauss.h"

#include <iostream>
#include <stdlib.h>

using namespace std;

namespace avg {
static ProfilingZone ProfilingZoneCapture ("Capture");
static ProfilingZone ProfilingZoneTracker ("Tracker");
static ProfilingZone ProfilingZoneHistory ("  History");
static ProfilingZone ProfilingZoneDistort ("  Distort");
static ProfilingZone ProfilingZoneHistogram ("  Histogram");
static ProfilingZone ProfilingZoneBandpass ("  Bandpass");
static ProfilingZone ProfilingZoneComps("  ConnectedComps");
static ProfilingZone ProfilingZoneUpdate("  Update");
static ProfilingZone ProfilingZoneDraw("  Draw");

TrackerThread::TrackerThread(IntRect ROI, 
        CameraPtr pCamera,
        BitmapPtr ppBitmaps[NUM_TRACKER_IMAGES],
        MutexPtr pMutex,
        CmdQueue& CmdQ,
        IBlobTarget *target,
        bool bSubtractHistory,
        TrackerConfig &config)
    : WorkerThread<TrackerThread>("Tracker", CmdQ),
      m_TouchThreshold(128),
      m_TrackThreshold(0),
      m_pMutex(pMutex),
      m_pCamera(pCamera),
      m_pTarget(target),
      m_bCreateDebugImages(false),
      m_bCreateFingerImage(false)
{
    if (bSubtractHistory) {
        m_pHistoryPreProcessor = HistoryPreProcessorPtr(
                new HistoryPreProcessor(ppBitmaps[1]->getSize(), 1));
    }
    setBitmaps(ROI, ppBitmaps);
    m_pDistorter = FilterDistortionPtr(new FilterDistortion(m_pBitmaps[0]->getSize(), 
            config.m_pTrafo));
}

TrackerThread::~TrackerThread()
{
}

bool TrackerThread::init()
{
    m_pCamera->open();
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
        if (m_bCreateDebugImages) {
            boost::mutex::scoped_lock Lock(*m_pMutex);
            *(m_pBitmaps[TRACKER_IMG_CAMERA]) = *pCamBmp;
            ScopeTimer Timer(ProfilingZoneHistogram);
            drawHistogram(m_pBitmaps[TRACKER_IMG_HISTOGRAM], pCamBmp);
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
            boost::mutex::scoped_lock Lock(*m_pMutex);
            if (m_bCreateFingerImage) {
                Pixel32 Black(0x00, 0x00, 0x00, 0x00);
                FilterFill<Pixel32>(Black).applyInPlace(
                        m_pBitmaps[TRACKER_IMG_FINGERS]);
            }
            BitmapPtr pBmpBandpass;
            if (m_TouchThreshold != 0) {
                {
                    ScopeTimer Timer(ProfilingZoneBandpass);
                    pBmpBandpass = FilterFastBandpass().apply(pCroppedBmp);
                }
                if (m_bCreateDebugImages) {
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

void TrackerThread::setConfig(TrackerConfig Config)
{
    if (Config.m_pTouch) {
        m_TouchThreshold = Config.m_pTouch->m_Threshold;
    } else {
        m_TouchThreshold = 0;
    }
    if (Config.m_pTrack) {
        m_TrackThreshold = Config.m_pTrack->m_Threshold;
    } else {
        m_TrackThreshold = 0;
    }
    if(m_pHistoryPreProcessor) {
        m_pHistoryPreProcessor->setInterval(Config.m_HistoryUpdateInterval);
    }
    if (m_pTrafo != Config.m_pTrafo) {
        m_pDistorter = FilterDistortionPtr(new FilterDistortion(m_pBitmaps[0]->getSize(), 
                Config.m_pTrafo));
        m_pTrafo = Config.m_pTrafo;
    }

    m_pCamera->setFeature("brightness", Config.m_Brightness);
    m_pCamera->setFeature("gamma", Config.m_Gamma);
    m_pCamera->setFeature("gain", Config.m_Gain);
    m_pCamera->setFeature("shutter", Config.m_Shutter);

    m_bCreateDebugImages = Config.m_bCreateDebugImages;
    m_bCreateFingerImage = Config.m_bCreateFingerImage;
}

void TrackerThread::setBitmaps(IntRect ROI, BitmapPtr ppBitmaps[NUM_TRACKER_IMAGES])
{
    m_ROI = ROI;
    for (int i=0; i<NUM_TRACKER_IMAGES; i++) {
        m_pBitmaps[i] = ppBitmaps[i];
    }
    if (m_pHistoryPreProcessor) {
        m_pHistoryPreProcessor = HistoryPreProcessorPtr(
                new HistoryPreProcessor(IntPoint(ROI.Width(), ROI.Height()), 
                        m_pHistoryPreProcessor->getInterval()));
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
    int Width = pDestBmp->getSize().x;
    int Stride = pDestBmp->getStride();
    int EndRow = 256;
    if (pDestBmp->getSize().y < 256) {
        EndRow = pDestBmp->getSize().y;
    }
    for (int i=0; i<EndRow; ++i) {
        int StartCol =Width-(*pHist)[i];
        if (StartCol < 0) { 
            StartCol = 0;
        }
        unsigned char * pDest = pDestBmp->getPixels()+Stride*i+StartCol;
        memset(pDest, 255, Width-StartCol);
    }
}

void TrackerThread::calcBlobs(BitmapPtr pTrackBmp, BitmapPtr pTouchBmp) {
    BlobListPtr pTrackComps;
    BlobListPtr pTouchComps;
    {
        ScopeTimer Timer(ProfilingZoneComps);
        pTrackComps = connected_components(pTrackBmp, m_TrackThreshold);
        pTouchComps = connected_components(pTouchBmp, m_TouchThreshold);
    }
    //    AVG_TRACE(Logger::EVENTS2, "connected components found "<<comps->size()<<" blobs.");
    //feed the IBlobTarget
    BitmapPtr pDestBmp;
    if (m_bCreateFingerImage) {
        pDestBmp = m_pBitmaps[TRACKER_IMG_FINGERS];
    }
    {
        ScopeTimer Timer(ProfilingZoneUpdate);
        m_pTarget->update(pTrackComps, pTrackBmp, m_TrackThreshold,
                pTouchComps, pTouchBmp, m_TouchThreshold, pDestBmp);
    }
}

}
