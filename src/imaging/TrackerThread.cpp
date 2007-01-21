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

#include "../base/Profiler.h"
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
static ProfilingZone ProfilingZoneCapture ("Capture", "Tracker");
static ProfilingZone ProfilingZoneTracker ("Tracker", "Tracker");
static ProfilingZone ProfilingZoneHistory ("  History", "Tracker");
static ProfilingZone ProfilingZoneDistort ("  Distort", "Tracker");
static ProfilingZone ProfilingZoneHistogram ("  Histogram", "Tracker");
static ProfilingZone ProfilingZoneBlur ("  Blur", "Tracker");
static ProfilingZone ProfilingZoneHighpass ("  Highpass", "Tracker");
static ProfilingZone ProfilingZoneComps("  ConnectedComps", "Tracker");
static ProfilingZone ProfilingZoneUpdate("  Update", "Tracker");

TrackerThread::TrackerThread(CameraPtr pCamera,
        BitmapPtr ppBitmaps[NUM_TRACKER_IMAGES],
        MutexPtr pMutex,
        CmdQueue& CmdQ,
        IBlobTarget *target,
        bool bSubtractHistory)
    : WorkerThread<TrackerThread>(CmdQ),
      m_Threshold(128),
      m_pMutex(pMutex),
      m_pCamera(pCamera),
      m_pTarget(target),
      m_bDebugEnabled(false)
{
    m_ROI = IntRect(IntPoint(0,0), ppBitmaps[1]->getSize());
    setBitmaps(m_ROI, ppBitmaps);
    if (bSubtractHistory) {
        m_pHistoryPreProcessor = HistoryPreProcessorPtr(
                new HistoryPreProcessor(m_pBitmaps[1]->getSize(), 1));
    }

    m_pDistorter = FilterDistortionPtr(new FilterDistortion(m_pBitmaps[0]->getSize(), 0.0, 0));
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
    {
        ScopeTimer Timer(ProfilingZoneTracker);
        if (m_bDebugEnabled) {
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
        if (m_bDebugEnabled) {
            boost::mutex::scoped_lock Lock(*m_pMutex);
            m_pBitmaps[TRACKER_IMG_DISTORTED]->copyPixels(*pCroppedBmp);
        }
        if (m_pHistoryPreProcessor) {
            ScopeTimer Timer(ProfilingZoneHistory);
            m_pHistoryPreProcessor->applyInPlace(pCroppedBmp);
        }
        if (m_bDebugEnabled) {
            boost::mutex::scoped_lock Lock(*m_pMutex);
            m_pBitmaps[TRACKER_IMG_NOHISTORY]->copyPixels(*pCroppedBmp);
        }
        BitmapPtr pBmpLowpass;
        {
            ScopeTimer Timer(ProfilingZoneBlur);
            pBmpLowpass = FilterBlur().apply(pCroppedBmp);
        }
        BitmapPtr pBmpHighpass;
        {
            ScopeTimer Timer(ProfilingZoneHighpass);
            pBmpHighpass = FilterHighpass().apply(pBmpLowpass);
        }
        if (m_bDebugEnabled) {
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
            m_pTarget->update(comps, m_pBitmaps[TRACKER_IMG_FINGERS]);
        }
    }
    Profiler::get().reset("Tracker");
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
    m_pDistorter = FilterDistortionPtr(new FilterDistortion(m_pBitmaps[0]->getSize(),
            Config.m_K1, Config.m_T));
        
    m_pCamera->setFeature("brightness", Config.m_Brightness);
    m_pCamera->setFeature("gamma", Config.m_Gamma);
    m_pCamera->setFeature("gain", Config.m_Gain);
    m_pCamera->setFeature("shutter", Config.m_Shutter);

    m_bDebugEnabled = Config.m_bDebug;
}

void TrackerThread::setBitmaps(IntRect ROI, BitmapPtr ppBitmaps[NUM_TRACKER_IMAGES])
{
    for (int i=0; i<NUM_TRACKER_IMAGES; i++) {
        m_pBitmaps[i] = ppBitmaps[i];
    }
    m_ROI = ROI;
    if (m_pHistoryPreProcessor) {
        m_pHistoryPreProcessor = HistoryPreProcessorPtr(
                new HistoryPreProcessor(IntPoint(m_ROI.Width(), m_ROI.Height()), 
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
