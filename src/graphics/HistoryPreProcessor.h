#ifndef _HistPreProcessor
#define _HistPreProcessor
#include "Filter.h"
#include "Bitmap.h"

#include <boost/shared_ptr.hpp>

namespace avg {
class HistoryPreProcessor: public Filter{
    public:
        HistoryPreProcessor(IntPoint dimensions, unsigned int frame_skip = 0);
        ~HistoryPreProcessor();
        virtual void applyInPlace(BitmapPtr pBmp);
        void reconfigure(unsigned short N, unsigned int frame_skip, bool reset);
    private:
        void updateHistory(BitmapPtr new_img);
        BitmapPtr substractHistory(BitmapPtr new_img);
        BitmapPtr m_pHistoryBmp;
        unsigned int m_FrameSkip;
        unsigned int m_FrameCounter;
        bool m_bHistoryInitialized;
};

typedef boost::shared_ptr<HistoryPreProcessor> HistoryPreProcessorPtr;
}
#endif
