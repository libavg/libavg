//
// $Id$
// 

#ifndef _AVGRegion_H_
#define _AVGRegion_H_

#include <paintlib/plrect.h>

#include <vector>

class AVGRegion 
{
	public:
        AVGRegion();
        virtual ~AVGRegion();

        void addRect(const PLRect& NewRect);
        void addRegion(const AVGRegion& NewRegion);
        void clear();

        int getNumRects() const;
        const PLRect& getRect(int i) const;

        void dump() const;

    private:
        std::vector<PLRect> m_Rects;
};

#endif //_AVGRegion_H_

