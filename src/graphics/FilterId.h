#ifndef _FilterId_
#define _FilterId

#include "Filter.h"
#include "Bitmap.h"

#include <boost/shared_ptr.hpp>

namespace avg {
class FilterId: public Filter{
    public:
        FilterId(){};
        virtual void applyInPlace(BitmapPtr img) {};
        virtual ~FilterId(){};
};

typedef boost::shared_ptr<FilterId> FilterIdPtr;
}
#endif
