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

#include <iostream>

using namespace std;

namespace avg {

TrackerThread::TrackerThread(std::string sDevice, double FrameRate, std::string sMode, 
        TouchInfoListPtr pTouchInfoList, BitmapPtr ppBitmaps[NUM_TRACKER_IMAGES],
        MutexPtr pMutex)
    : m_sDevice(sDevice),
      m_FrameRate(FrameRate),
      m_sMode(sMode),
      m_pTouchInfoList(pTouchInfoList),
      m_pMutex(pMutex),
      m_bShouldStop(false)
{
    for (int i=0; i<NUM_TRACKER_IMAGES; i++) {
        m_pBitmaps[i] = ppBitmaps[i];
    }
    m_pHistoryBmp = BitmapPtr(new Bitmap(ppBitmaps[0]->getSize(), I16));
    memset(m_pHistoryBmp->getPixels(), 0, 
            m_pHistoryBmp->getSize().y*m_pHistoryBmp->getStride());
}

TrackerThread::~TrackerThread()
{
}

void TrackerThread::operator()()
{
    open();
    while (!m_bShouldStop) {
        track();
        checkMessages();
    }
    close();
}

void TrackerThread::open()
{
    m_pCamera = CameraPtr(new Camera(m_sDevice, m_FrameRate, m_sMode, false));
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
}

void TrackerThread::checkMessages()
{
}

void TrackerThread::calcHistory()
{
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
}

}
