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
//
//  Original author of this file is igor@c-base.org.
//

#ifndef _HistoryPreProcessor_H_
#define _HistoryPreProcessor_H_

#include "Filter.h"
#include "Bitmap.h"

#include <boost/shared_ptr.hpp>

namespace avg {

class HistoryPreProcessor: public Filter
{
    public:
        HistoryPreProcessor(IntPoint dimensions, unsigned int UpdateInterval = 1);
        virtual ~HistoryPreProcessor();
        virtual void applyInPlace(BitmapPtr pBmp);
        void setInterval(unsigned int UpdateInterval);
        unsigned int getInterval(); 
        void reset();

    private:
        HistoryPreProcessor(const HistoryPreProcessor&) {};
        void updateHistory(BitmapPtr new_img);
        void normalizeHistogram(BitmapPtr pBmp, unsigned char Max);
        template<int SPEED> void calcAvg(BitmapPtr pNewBmp); 

        BitmapPtr m_pHistoryBmp;
        unsigned int m_FrameCounter;
        unsigned int m_UpdateInterval;
        typedef enum {NO_IMAGE, INITIALIZING, NORMAL} State;
        State m_State;
        int m_NumInitImages;
};

template<int SPEED>
void HistoryPreProcessor::calcAvg(BitmapPtr pNewBmp) {
    const int SRC_NUMERATOR = SPEED-1;
    const int SRC_DENOMINATOR = SPEED;
    const int DEST_FACTOR = 256/SPEED;
    const unsigned char * pSrc = pNewBmp->getPixels();
    unsigned short * pDest = (unsigned short*)(m_pHistoryBmp->getPixels());
    int DestStride = m_pHistoryBmp->getStride()/m_pHistoryBmp->getBytesPerPixel();
    IntPoint Size = m_pHistoryBmp->getSize();
    for (int y=0; y<Size.y; y++) {
        const unsigned char * pSrcPixel = pSrc;
        unsigned short * pDestPixel = pDest;
        for (int x=0; x<Size.x; x++) {
            int t = SRC_NUMERATOR*int(*pDestPixel)/SRC_DENOMINATOR;
            *pDestPixel = (t) + int(*pSrcPixel)*DEST_FACTOR;
            pDestPixel++;
            pSrcPixel++;
        }
        pDest += DestStride;
        pSrc += pNewBmp->getStride();
    }

}

typedef boost::shared_ptr<HistoryPreProcessor> HistoryPreProcessorPtr;

}
#endif
