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
        void updateHistory(BitmapPtr new_img);
        void normalizeHistogram(BitmapPtr pBmp, unsigned char Max);
        BitmapPtr m_pHistoryBmp;
        unsigned int m_FrameCounter;
        unsigned int m_UpdateInterval;
        //interpretation of m_HistoryInitialized
        //==0 normal operation
        //<0 add continue resetting the history while ignoring UpdateInterval, then m_HistoryInitialized++;
        //>0 copy a fresh frame into m_pHistoryBmp and reverse sign of m_HistoryInitialized
        // TODO: Clean this up.
        int m_HistoryInitialized;
};

typedef boost::shared_ptr<HistoryPreProcessor> HistoryPreProcessorPtr;

}
#endif
