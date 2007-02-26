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
#include "ConnectedComps.h"
#include "FilterDistortion.h"

#include "../base/Logger.h"
#include "../base/ProfilingZone.h"
#include "../base/ScopeTimer.h"

#include "../graphics/Filter.h"
#include "../graphics/Filterfill.h"
#include "../graphics/FilterHighpass.h"
#include "../graphics/FilterBlur.h"

#include <iostream>
#include <stdlib.h>

using namespace std;

namespace avg {
static ProfilingZone ProfilingZoneCapture ("Capture");
static ProfilingZone ProfilingZoneTracker ("Tracker");
static ProfilingZone ProfilingZoneHistory ("  History");
static ProfilingZone ProfilingZoneDistort ("  Distort");
static ProfilingZone ProfilingZoneHistogram ("  Histogram");
static ProfilingZone ProfilingZoneBlur ("  Blur");
static ProfilingZone ProfilingZoneHighpass ("  Highpass");
static ProfilingZone ProfilingZoneComps("  ConnectedComps");
static ProfilingZone ProfilingZoneUpdate("  Update");

TrackerThread::TrackerThread(CameraPtr pCamera,
        BitmapPtr ppBitmaps[NUM_TRACKER_IMAGES],
        MutexPtr pMutex,
        CmdQueue& CmdQ,
        IBlobTarget *target,
        bool bSubtractHistory,
        TrackerConfig &config)
    : WorkerThread<TrackerThread>("Tracker", CmdQ),
      m_Threshold(128),
      m_pMutex(pMutex),
      m_pCamera(pCamera),
      m_pTarget(target),
      m_bCreateDebugImages(false),
      m_bCreateFingerImage(false)
{
    setBitmaps(ppBitmaps);
    if (bSubtractHistory) {
        m_pHistoryPreProcessor = HistoryPreProcessorPtr(
                new HistoryPreProcessor(m_pBitmaps[1]->getSize(), 1));
    }

    m_pDistorter = FilterDistortionPtr(new FilterDistortion(m_pBitmaps[0]->getSize(), config.m_pTrafo));
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
        if (m_bCreateDebugImages) {
            boost::mutex::scoped_lock Lock(*m_pMutex);
            m_pBitmaps[TRACKER_IMG_DISTORTED]->copyPixels(*pDistortedBmp);
        }
        if (m_pHistoryPreProcessor) {
            ScopeTimer Timer(ProfilingZoneHistory);
            m_pHistoryPreProcessor->applyInPlace(pDistortedBmp);
        }
        if (m_bCreateDebugImages) {
            boost::mutex::scoped_lock Lock(*m_pMutex);
            m_pBitmaps[TRACKER_IMG_NOHISTORY]->copyPixels(*pDistortedBmp);
        }
        BitmapPtr pBmpLowpass;
        {
            ScopeTimer Timer(ProfilingZoneBlur);
            pBmpLowpass = FilterBlur().apply(pDistortedBmp);
        }
        BitmapPtr pBmpHighpass;
        {
            ScopeTimer Timer(ProfilingZoneHighpass);
            pBmpHighpass = FilterHighpass().apply(pBmpLowpass);
        }
        if (m_bCreateDebugImages) {
            boost::mutex::scoped_lock Lock(*m_pMutex);
            *(m_pBitmaps[TRACKER_IMG_HIGHPASS]) = *pBmpHighpass;
        }
        //get bloblist
        //
        BlobListPtr comps;
        {
            ScopeTimer Timer(ProfilingZoneComps);
            comps = connected_components(pBmpHighpass, m_Threshold);
        }
        //    AVG_TRACE(Logger::EVENTS2, "connected components found "<<comps->size()<<" blobs.");
        //feed the IBlobTarget
        {
            boost::mutex::scoped_lock Lock(*m_pMutex);
            ScopeTimer Timer(ProfilingZoneUpdate);
            if (m_bCreateFingerImage) {
                m_pTarget->update(comps, m_pBitmaps[TRACKER_IMG_FINGERS]);
            } else {
                m_pTarget->update(comps, BitmapPtr());
            }
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
    m_Threshold = Config.m_Threshold;
    if(m_pHistoryPreProcessor) {
        m_pHistoryPreProcessor->setInterval(Config.m_HistoryUpdateInterval);
    }
    if (m_pTrafo != Config.m_pTrafo) {
        m_pDistorter = FilterDistortionPtr(new FilterDistortion(m_pBitmaps[0]->getSize(), Config.m_pTrafo));
        m_pTrafo = Config.m_pTrafo;
    }
        
    m_pCamera->setFeature("brightness", Config.m_Brightness);
    m_pCamera->setFeature("gamma", Config.m_Gamma);
    m_pCamera->setFeature("gain", Config.m_Gain);
    m_pCamera->setFeature("shutter", Config.m_Shutter);

    m_bCreateDebugImages = Config.m_bCreateDebugImages;
    m_bCreateFingerImage = Config.m_bCreateFingerImage;
}

void TrackerThread::setBitmaps(BitmapPtr ppBitmaps[NUM_TRACKER_IMAGES])
{
    for (int i=0; i<NUM_TRACKER_IMAGES; i++) {
        m_pBitmaps[i] = ppBitmaps[i];
    }
    if (m_pHistoryPreProcessor) {
        IntPoint ROI = m_pBitmaps[0]->getSize();
        m_pHistoryPreProcessor = HistoryPreProcessorPtr(
                new HistoryPreProcessor(IntPoint(ROI.x, ROI.y), 
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

}
