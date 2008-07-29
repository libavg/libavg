//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2006 Ulrich von Zadow
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

#ifndef _BicubicSpline_H_
#define _BicubicSpline_H_

#include "Point.h"

#include <boost/shared_ptr.hpp>
#include <vector>

namespace avg {

class BicubicSpline {
public:
    BicubicSpline(const std::vector<double>& x, const std::vector<double>& y,
            const std::vector<std::vector<double> >& f);
    virtual ~BicubicSpline();

    double interpolate(const DPoint& orig);

private:
    double getX(int j);
    double getY(int i);
    double getF(int i, int j);
    void getCoeffs(int i, int j, std::vector<std::vector<double> > & coeffs);

    std::vector<double> m_X;
    std::vector<double> m_Y;
    std::vector<std::vector<double> > m_F;
    std::vector<std::vector<double> > m_Fdx;
    std::vector<std::vector<double> > m_Fdy;
    std::vector<std::vector<double> > m_Fdxy;
};

typedef boost::shared_ptr<BicubicSpline> BicubicSplinePtr;

}

#endif 



