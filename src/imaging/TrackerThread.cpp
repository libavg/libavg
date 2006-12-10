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
#include <iostream>
#include <stdlib.h>

using namespace std;

namespace avg {

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
        boost::mutex::scoped_lock Lock(*m_pMutex);
        *(m_pBitmaps[TRACKER_IMG_CAMERA]) = *pTempBmp;
    }
    if (m_pHistoryPreProcessor) {
        m_pHistoryPreProcessor->applyInPlace(pTempBmp);
    }
    {
        boost::mutex::scoped_lock Lock(*m_pMutex);
        m_pBitmaps[TRACKER_IMG_NOHISTORY]->copyPixels(*pTempBmp);
    }
    //get bloblist
    //BlobListPtr comps = connected_components(m_pBitmaps[TRACKER_IMG_NOHISTORY], m_Threshold);
    BlobListPtr comps = connected_components(pTempBmp, m_Threshold);
//    AVG_TRACE(Logger::EVENTS2, "connected components found "<<comps->size()<<" blobs.");
    //feed the IBlobTarget
    m_pTarget->update(comps);
    return true;
}

void TrackerThread::deinit()
{
    m_pCamera->close();
}

void TrackerThread::setConfig(TrackerConfig Config)
{
    m_Threshold = Config.m_Threshold;
    m_pHistoryPreProcessor->setInterval(Config.m_HistoryUpdateInterval);
    m_pCamera->setFeature("brightness", Config.m_Brightness);
    m_pCamera->setFeature("exposure", Config.m_Exposure);
    m_pCamera->setFeature("gain", Config.m_Gain);
    m_pCamera->setFeature("shutter", Config.m_Shutter);
}

void TrackerThread::resetHistory()
{
    m_pHistoryPreProcessor->reset();
}

}
