#ifndef _FilterDistort
#define _FilterDistort

#include "CoordTransformer.h"

#include "../graphics/Filter.h"
#include "../graphics/Point.h"
#include "../graphics/Rect.h"

#include <boost/shared_ptr.hpp>

namespace avg {
    class FilterDistortion:public Filter {
        public:
            FilterDistortion(IntPoint srcSize,double K1, double T);
            BitmapPtr apply (BitmapPtr pBmpSource);
        private:
            IntRect m_srcRect;
            CoordTransformer m_trafo;
            IntPoint *m_pMap;
    };

    typedef boost::shared_ptr<FilterDistortion> FilterDistortionPtr;
}

#endif
