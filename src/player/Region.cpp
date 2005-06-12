//
// $Id$
// 

#include "Region.h"

#include <iostream>

using namespace std;

namespace avg {

Region::Region ()
{
}

Region::~Region ()
{
}

void Region::addRect(const DRect& NewRect) {
    if (NewRect.Width() <= 0 || NewRect.Height() <= 0) {
        // Ignore empty rectangles.
        return;
    }
    DRect CurRect(NewRect);
    bool bFound = false;
    do {
        bFound = false;
        std::vector<DRect>::iterator it;
        for (it = m_Rects.begin(); it != m_Rects.end() && !bFound; it++) {
            if ((*it).Intersects(CurRect)) {
                CurRect.Expand(*it); 
                m_Rects.erase(it);
                bFound = true;
            }
        }
    } while (bFound);
    m_Rects.push_back(CurRect);
}

void Region::addRegion(const Region& NewRegion) {
    for (int i = 0; i<NewRegion.getNumRects(); i++) {
        addRect (NewRegion.getRect(i));
    }
}

void Region::clear()
{
    m_Rects.clear();
}

int Region::getNumRects() const {
    return m_Rects.size();

}

const DRect& Region::getRect(int i) const {
    return m_Rects[i];
}

void Region::dump() const {
    for (unsigned int i = 0; i<m_Rects.size(); i++) {
        const DRect & r = m_Rects[i];
        cerr << "[" << r.tl.x << "x" << r.tl.y << ", " << 
                 r.br.x << "x" << r.br.y << "]" << endl;
    }
    cerr << endl;
}

}
