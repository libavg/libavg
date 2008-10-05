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

#ifndef _BezierCurve_H_
#define _BezierCurve_H_

#include "Point.h"

#include <boost/shared_ptr.hpp>
#include <vector>

namespace avg {

class BezierCurve {
public:
    BezierCurve(const DPoint& p0, const DPoint& p1, const DPoint& p2, const DPoint& p3);

    DPoint interpolate(double t) const;
    DPoint getDeriv(double t) const;

private:
    DPoint m_P0;
    DPoint m_P1;
    DPoint m_P2;
    DPoint m_P3;
};

typedef boost::shared_ptr<BezierCurve> BezierCurvePtr;

}

#endif 



