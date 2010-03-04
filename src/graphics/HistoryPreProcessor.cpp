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
//
//  Original author of this file is igor@c-base.org.
//

#include "HistoryPreProcessor.h"

#include "Bitmap.h"
#include "Filterfill.h"

#include "../base/Exception.h"

#include <iostream>
#include <sstream>

using namespace std;

#define FAST_HISTORY_SPEED 16

namespace avg {
    
HistoryPreProcessor::HistoryPreProcessor(IntPoint dimensions, 
        unsigned int UpdateInterval, bool bBrighter)
    : m_FrameCounter(0),
      m_UpdateInterval(UpdateInterval),
      m_bBrighter(bBrighter)
{
    m_pHistoryBmp = BitmapPtr(new Bitmap(dimensions, I16));
    reset();
}

HistoryPreProcessor::~HistoryPreProcessor()
{
}
void HistoryPreProcessor::setInterval(unsigned int UpdateInterval)
{
    m_FrameCounter = 0;
    m_UpdateInterval = UpdateInterval;
}

unsigned int HistoryPreProcessor::getInterval()
{
    return m_UpdateInterval;
}

void HistoryPreProcessor::reset()
{
    m_State = NO_IMAGE;
}

void HistoryPreProcessor::updateHistory(BitmapPtr pNewBmp)
{
    AVG_ASSERT(pNewBmp->getSize() == m_pHistoryBmp->getSize());
    switch (m_State) {
        case NO_IMAGE:
            m_pHistoryBmp->copyPixels(*pNewBmp);
            m_State = INITIALIZING;
            m_NumInitImages = 0;
            break;
        case INITIALIZING:
            calcAvg<FAST_HISTORY_SPEED>(pNewBmp);
            m_NumInitImages++;
            if (m_NumInitImages == FAST_HISTORY_SPEED*2) {
                m_State = NORMAL;
            }
            break;
        case NORMAL:
            if (m_FrameCounter < m_UpdateInterval-1) {
                m_FrameCounter++;
            } else {
                m_FrameCounter = 0;
                calcAvg<256>(pNewBmp);
            }
            break;
    }
}

void HistoryPreProcessor::applyInPlace(BitmapPtr img)
{
    updateHistory(img);
    unsigned short * pSrc = (unsigned short*)m_pHistoryBmp->getPixels();
    int SrcStride = m_pHistoryBmp->getStride()/m_pHistoryBmp->getBytesPerPixel();
    int DestStride = img->getStride();
    unsigned char * pDest = img->getPixels();
    IntPoint Size = img->getSize();
//    unsigned char Max = 0;
    for (int y=0; y<Size.y; y++) {
        const unsigned short * pSrcPixel = pSrc;
        unsigned char * pDestPixel = pDest;
        if (m_bBrighter) {
            for (int x=0; x<Size.x; x++) {
                unsigned char Src = *pSrcPixel/256;
                if ((*pDestPixel)>Src) {
                    *pDestPixel = *pDestPixel-Src;
                } else {
                    *pDestPixel = 0;
                }
                pDestPixel++;
                pSrcPixel++;
            }
        } else {
            for (int x=0; x<Size.x; x++) {
                unsigned char Src = *pSrcPixel/256;
                if ((*pDestPixel)<Src) {
                    *pDestPixel = Src-*pDestPixel;
                } else {
                    *pDestPixel = 0;
                }
                pDestPixel++;
                pSrcPixel++;
            }
        }
        pDest += DestStride;
        pSrc += SrcStride;
    }
}

// Fast pseudo-normalization with an integer factor.
void HistoryPreProcessor::normalizeHistogram(BitmapPtr pBmp, unsigned char Max)
{
    if (Max < 128) {
        Max = 128;
    }
    int Factor = int(256.0/Max);
    unsigned char * pLine = pBmp->getPixels();
    IntPoint Size = pBmp->getSize();
    int Stride = pBmp->getStride();
    for (int y=0; y<Size.y; y++) {
        unsigned char * pPixel = pLine;
        for (int x=0; x<Size.x; x++) {
            *pPixel *= Factor;
            pPixel++;
        }
        pLine += Stride;
    }
}

}

