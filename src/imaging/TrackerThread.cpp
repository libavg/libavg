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
#include "../base/Logger.h"
#include "../graphics/Filter.h"
#include "../graphics/Filterfill.h"
#include "FilterDistortion.h"
#include <iostream>
#include <stdlib.h>

#include "../base/ProfilingZone.h"
#include "../base/ScopeTimer.h"

using namespace std;

namespace avg {
static ProfilingZone ProfilingZoneTracker ("Tracker", "Tracker");
static ProfilingZone ProfilingZoneHistory ("  History", "Tracker");
static ProfilingZone ProfilingZoneDistort ("  Distort", "Tracker");
static ProfilingZone ProfilingZoneHistogram ("  Histogram", "Tracker");
static ProfilingZone ProfilingZoneComps("  ConnectedComps", "Tracker");

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
      m_pTarget(target)
{
    for (int i=0; i<NUM_TRACKER_IMAGES; i++) {
        m_pBitmaps[i] = ppBitmaps[i];
    }
    if (bSubtractHistory) {
        m_pHistoryPreProcessor = HistoryPreProcessorPtr(
                new HistoryPreProcessor(m_pBitmaps[0]->getSize(), 1));
    }

    m_pDistorter = FilterDistortionPtr(new FilterDistortion(m_pBitmaps[0]->getSize(), 0, 0));
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
    BitmapPtr pTempBmp = m_pCamera->getImage(true);
    BitmapPtr pTempBmp1;
    while (pTempBmp1 = m_pCamera->getImage(false)) {
        pTempBmp = pTempBmp1;
    }
    {
        ScopeTimer Timer(ProfilingZoneTracker);
        {
            boost::mutex::scoped_lock Lock(*m_pMutex);
            *(m_pBitmaps[TRACKER_IMG_CAMERA]) = *pTempBmp;
        }
        {
            if (m_pHistoryPreProcessor) {
                ScopeTimer Timer(ProfilingZoneHistory);
                m_pHistoryPreProcessor->applyInPlace(pTempBmp);
            }
            {
                ScopeTimer Timer(ProfilingZoneDistort);
                pTempBmp1 = m_pDistorter->apply(pTempBmp);
            }
        }
        {
            boost::mutex::scoped_lock Lock(*m_pMutex);
            m_pBitmaps[TRACKER_IMG_NOHISTORY]->copyPixels(*pTempBmp1);
        }
        {
            ScopeTimer Timer(ProfilingZoneHistogram);
            boost::mutex::scoped_lock Lock(*m_pMutex);
            drawHistogram(m_pBitmaps[TRACKER_IMG_HISTOGRAM], 
                    m_pBitmaps[TRACKER_IMG_NOHISTORY]);
        }
        //get bloblist
        //
        BlobListPtr comps;
        {
            ScopeTimer Timer(ProfilingZoneComps);
            comps = connected_components(pTempBmp1, m_Threshold);
        }
        //    AVG_TRACE(Logger::EVENTS2, "connected components found "<<comps->size()<<" blobs.");
        //feed the IBlobTarget
        {
            boost::mutex::scoped_lock Lock(*m_pMutex);
            m_pTarget->update(comps);
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
    if(m_pHistoryPreProcessor)
        m_pHistoryPreProcessor->setInterval(Config.m_HistoryUpdateInterval);
    m_pCamera->setFeature("brightness", Config.m_Brightness);
    m_pCamera->setFeature("exposure", Config.m_Exposure);
    m_pCamera->setFeature("gain", Config.m_Gain);
    m_pCamera->setFeature("shutter", Config.m_Shutter);
}

void TrackerThread::resetHistory()
{
    if(m_pHistoryPreProcessor)
        m_pHistoryPreProcessor->reset();
}
        
void TrackerThread::drawHistogram(BitmapPtr pDestBmp, BitmapPtr pSrcBmp)
{
    HistogramPtr pHist = pSrcBmp->getHistogram();
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
    int Height = pDestBmp->getSize().y;
    int Stride = pDestBmp->getStride();
    int EndCol = 256;
    if (pDestBmp->getSize().x < 256) {
        EndCol = pDestBmp->getSize().x;
    }
    for (int i=0; i<EndCol; ++i) {
        int StartLine =Height-(*pHist)[i];
        if (StartLine < 0) { 
            StartLine = 0;
        }
        unsigned char * pDest = pDestBmp->getPixels()+Stride*StartLine+i;
        for (int y=StartLine-1; y < Height-1; ++y) {
            *pDest = 255;
            pDest += Stride;
        }
        
    }
}

}
