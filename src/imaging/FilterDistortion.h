#ifndef _FilterDistort
#define _FilterDistort

#include "../graphics/Filter.h"
#include "../graphics/Point.h"
#include "../graphics/Rect.h"
#include "CoordTransformer.h"
#include <boost/multi_array.hpp>
#include <boost/shared_ptr.hpp>

namespace avg {

    typedef boost::multi_array<IntPoint,2> pixel_map;
    typedef pixel_map::index index;

    class FilterDistortion:public Filter {
        public:
            FilterDistortion(IntPoint srcSize,double K1, double T);
            BitmapPtr apply (BitmapPtr pBmpSource);
        private:
            IntRect m_srcRect;
            CoordTransformer m_trafo;
            pixel_map *m_pMap;
    };

    typedef boost::shared_ptr<FilterDistortion> FilterDistortionPtr;
}

#endif
