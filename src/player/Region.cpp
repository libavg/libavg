//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2008 Ulrich von Zadow
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  Current versions can be found at www.libavg.de
//

#include "Region.h"

#include "../base/ObjectCounter.h"

#include <iostream>

using namespace std;

namespace avg {

Region::Region ()
{
    ObjectCounter::get()->incRef(&typeid(*this));
}

Region::~Region ()
{
    ObjectCounter::get()->decRef(&typeid(*this));
}

void Region::addRect(const DRect& NewRect) {
    if (NewRect.width() <= 0 || NewRect.height() <= 0) {
        // Ignore empty rectangles.
        return;
    }
    DRect CurRect(NewRect);
    bool bFound = false;
    do {
        bFound = false;
        std::vector<DRect>::iterator it;
        for (it = m_Rects.begin(); it != m_Rects.end(); it++) {
            if ((*it).intersects(CurRect)) {
                CurRect.expand(*it); 
                m_Rects.erase(it);
                bFound = true;
                break;
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
    return int(m_Rects.size());

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
