//
// $Id$
// 

#include "AVGRegion.h"

#include <iostream>

using namespace std;


AVGRegion::AVGRegion ()
{
}

AVGRegion::~AVGRegion ()
{
}

void AVGRegion::addRect(const PLRect& NewRect) {
    if (NewRect.Width() == 0 || NewRect.Height() == 0) {
        // Ignore empty rectangles.
        return;
    }
    PLRect CurRect(NewRect);
    bool bFound = false;
    do {
        bFound = false;
        std::vector<PLRect>::iterator it;
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

void AVGRegion::addRegion(const AVGRegion& NewRegion) {
    for (int i = 0; i<NewRegion.getNumRects(); i++) {
        addRect (NewRegion.getRect(i));
    }
}

void AVGRegion::clear()
{
    m_Rects.clear();
}

int AVGRegion::getNumRects() const {
    return m_Rects.size();

}

const PLRect& AVGRegion::getRect(int i) const {
    return m_Rects[i];
}

void AVGRegion::dump() const {
    for (int i = 0; i<m_Rects.size(); i++) {
        const PLRect & r = m_Rects[i];
        cerr << "[" << r.tl.x << "x" << r.tl.y << ", " << 
                 r.br.x << "x" << r.br.y << "]" << endl;
    }
}

