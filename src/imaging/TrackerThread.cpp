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
#include "../graphics/Filterfill.h"
#include <iostream>
#include <stdlib.h>

using namespace std;

namespace avg {

TrackerThread::TrackerThread(CameraPtr pCamera, int Threshold, 
        BitmapPtr ppBitmaps[NUM_TRACKER_IMAGES],
        MutexPtr pMutex,
        TrackerCmdQueuePtr pCmdQ,
        IBlobTarget *target
        )
    : m_Threshold(Threshold),
      m_pMutex(pMutex),
      m_bHistoryInitialized(false),
      m_pCamera(pCamera),
      m_pCmdQ(pCmdQ),
      m_pTarget(target),
      m_bShouldStop(false)

{
    for (int i=0; i<NUM_TRACKER_IMAGES; i++) {
        m_pBitmaps[i] = ppBitmaps[i];
    }
    m_pHistoryBmp = BitmapPtr(new Bitmap(ppBitmaps[0]->getSize(), I16));
    memset(m_pHistoryBmp->getPixels(), 0, 
            m_pHistoryBmp->getSize().y*m_pHistoryBmp->getStride());
        AVG_TRACE(Logger::CONFIG, "Tracker thread started.");
}

TrackerThread::~TrackerThread()
{
}

void TrackerThread::operator()()
{
    AVG_TRACE(Logger::CONFIG, "Tracker thread stopped.");
    open();
    while (!m_bShouldStop) {
        track();
        checkMessages();
    }
    close();
}

void TrackerThread::open()
{
    m_pCamera->open();
}

void TrackerThread::close()
{
    m_pCamera->close();
}

void TrackerThread::track()
{
    BitmapPtr pTempBmp = m_pCamera->getImage(true);
    {
        boost::mutex::scoped_lock Lock(*m_pMutex);
        *(m_pBitmaps[TRACKER_IMG_CAMERA]) = *pTempBmp;
    }
    calcHistory();
    {
        boost::mutex::scoped_lock Lock(*m_pMutex);
        m_pBitmaps[TRACKER_IMG_HISTORY]->copyPixels(*m_pHistoryBmp);
    }
    pTempBmp = subtractHistory();
    {
        boost::mutex::scoped_lock Lock(*m_pMutex);
        m_pBitmaps[TRACKER_IMG_NOHISTORY]->copyPixels(*pTempBmp);
    }
    //get bloblist
    BlobListPtr comps = connected_components(m_pBitmaps[TRACKER_IMG_NOHISTORY], m_Threshold);
    {
        boost::mutex::scoped_lock Lock(*m_pMutex);
        FilterFill<Pixel8>(0x00).applyInPlace(m_pBitmaps[TRACKER_IMG_COMPONENTS]);
        for (BlobList::iterator it=comps->begin(); it != comps->end(); ++it) {
            render(&(*m_pBitmaps[TRACKER_IMG_COMPONENTS]), *it, 0xFF);
        }
    }
    AVG_TRACE(Logger::EVENTS2, "connected components found "<<comps->size()<<" blobs.");
    //feed the IBlobTarget
    m_pTarget->update(comps);
}

void TrackerThread::checkMessages()
{
    try {
        // This loop always ends in an exception when the Queue is empty.
        while (true) {
            TrackerCmdPtr pCmd = m_pCmdQ->pop(false);
            pCmd->execute(this);
        }
    } catch (Exception& ex) {
    }
}

void TrackerThread::stop()
{
    m_bShouldStop = true;
}

void TrackerThread::setThreshold(int Threshold) 
{
    m_Threshold = Threshold;
}

void TrackerThread::setBrightness(int Brightness) 
{
    m_pCamera->setFeature("brightness", Brightness);
}

void TrackerThread::setExposure(int Exposure) 
{
    m_pCamera->setFeature("exposure", Exposure);
}

void TrackerThread::calcHistory()
{
    if (m_bHistoryInitialized) {
        const unsigned char * pSrc = m_pBitmaps[TRACKER_IMG_CAMERA]->getPixels();
        unsigned short * pDest = (unsigned short *)(m_pHistoryBmp->getPixels());
        int DestStride = m_pHistoryBmp->getStride()/m_pHistoryBmp->getBytesPerPixel();
        IntPoint Size = m_pHistoryBmp->getSize();
        for (int y=0; y<Size.y; y++) {
            const unsigned char * pSrcPixel = pSrc;
            unsigned short * pDestPixel = pDest;
            for (int x=0; x<Size.x; x++) {
                *pDestPixel = (255*((long)*pDestPixel))/256 + *pSrcPixel;
                pDestPixel++;
                pSrcPixel++;
            }
            pDest += DestStride;
            pSrc += m_pBitmaps[TRACKER_IMG_CAMERA]->getStride();
        }
    } else {
        m_pHistoryBmp->copyPixels(*m_pBitmaps[TRACKER_IMG_CAMERA]);
        m_bHistoryInitialized = true;
    }
}

BitmapPtr TrackerThread::subtractHistory()
{
    BitmapPtr pNoHistoryBmp(new Bitmap(*m_pBitmaps[TRACKER_IMG_CAMERA]));
    const unsigned char * pSrc = m_pBitmaps[TRACKER_IMG_HISTORY]->getPixels();
    unsigned char * pDest = pNoHistoryBmp->getPixels();
    IntPoint Size = pNoHistoryBmp->getSize();
    for (int y=0; y<Size.y; y++) {
        const unsigned char * pSrcPixel = pSrc;
        unsigned char * pDestPixel = pDest;
        for (int x=0; x<Size.x; x++) {
            *pDestPixel = (unsigned char)abs(*pDestPixel - *pSrcPixel);
            pDestPixel++;
            pSrcPixel++;
        }
        pDest += pNoHistoryBmp->getStride();
        pSrc += m_pBitmaps[TRACKER_IMG_CAMERA]->getStride();
    }
    return pNoHistoryBmp;
}

}
