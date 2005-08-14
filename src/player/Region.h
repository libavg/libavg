//
// $Id$
// 

#ifndef _Region_H_
#define _Region_H_

#include "../graphics/Rect.h"

#include <vector>

namespace avg {

class Region 
{
    public:
        Region();
        virtual ~Region();

        void addRect(const DRect& NewRect);
        void addRegion(const Region& NewRegion);
        void clear();

        int getNumRects() const;
        const DRect& getRect(int i) const;

        void dump() const;

    private:
        std::vector<DRect > m_Rects;
};

}

#endif //_Region_H_

