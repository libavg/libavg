#ifndef _HistPreProcessor
#define _HistPreProcessor
#include "Filter.h"
#include "Bitmap.h"

#include <boost/shared_ptr.hpp>

namespace avg {
class HistoryPreProcessor: public Filter{
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
        int m_HistoryInitialized;
};

typedef boost::shared_ptr<HistoryPreProcessor> HistoryPreProcessorPtr;
}
#endif
