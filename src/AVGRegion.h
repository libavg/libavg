//
// $Id$
// 

#ifndef _AVGRegion_H_
#define _AVGRegion_H_

#include "AVGRect.h"

#include <vector>

class AVGRegion 
{
    public:
        AVGRegion();
        virtual ~AVGRegion();

        void addRect(const AVGDRect& NewRect);
        void addRegion(const AVGRegion& NewRegion);
        void clear();

        int getNumRects() const;
        const AVGDRect& getRect(int i) const;

        void dump() const;

    private:
        std::vector<AVGDRect > m_Rects;
};

#endif //_AVGRegion_H_

