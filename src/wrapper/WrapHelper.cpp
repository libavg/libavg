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

#include "WrapHelper.h"

using namespace avg;
using namespace std;

namespace DPointHelper
{
    int len(const DPoint&) 
    {
        return 2;
    }

    double getX(const DPoint& pt)
    {
        return pt.x;
    }

    double getY(const DPoint& pt)
    {
        return pt.y;
    }

    void setX(DPoint& pt, double val)
    {
        pt.x = val;
    }

    void setY(DPoint& pt, double val)
    {
        pt.y = val;
    }

    void checkItemRange(int i) {
        if (i!=0 && i!=1) {
            throw std::out_of_range("Index out of range for Point2D. Must be 0 or 1.");
        }
    }

    double getItem(const DPoint& pt, int i)
    {
        checkItemRange(i);
        if (i==0) {
            return pt.x;
        } else {
            return pt.y;
        }
    }

    void setItem(DPoint& pt, int i, double val)
    {
        checkItemRange(i);
        if (i==0) {
            pt.x = val;
        } else {
            pt.y = val;
        }
    }

    string str(const DPoint& pt)
    {
        stringstream st;
        st << "(" << pt.x << "," << pt.y << ")";
        return st.str();
    }

    string repr(const DPoint& pt)
    {
        stringstream st;
        st << "avg.Point2D(" << pt.x << "," << pt.y << ")";
        return st.str();
    }

    long getHash(const DPoint& pt)
    {
        // Wild guess at what could constitute a good hash function.
        // Will generate very bad hashes if most values are in a range < 0.1,
        // but this is meant for pixel values anyway, right? ;-).
        return long(pt.x*42+pt.y*23);
    }
}

// The ConstDPoint stuff is there so that DPoint attributes behave sensibly. That is,
// node.pos.x = 30 causes an error instead of failing silently.
ConstDPoint::ConstDPoint()
{
}

ConstDPoint::ConstDPoint(const DPoint& other)
{
    x = other.x;
    y = other.y;
}

ConstDPoint::operator DPoint() const
{
    return DPoint(x,y);
}


