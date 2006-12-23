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

#ifndef _CoordTransformer_H_
#define _CoordTransformer_H_

#include "../graphics/Point.h"
#include "../graphics/Rect.h"

#include <boost/shared_ptr.hpp>

namespace avg {

class CoordTransformer {
    public:
        CoordTransformer(IntRect srcRect, double K1, double T, double RescaleFactor = 1);
        virtual ~CoordTransformer();

        void load(const std::string & sFilename);
        void save(const std::string & sFilename);

        DPoint transform_point(const DPoint & pt); //(x,y) -> (x', y')
        DPoint inverse_transform_point(const DPoint & pt); //(x,y) -> (x', y')
        //l_x = srcRect.width
        //l_y = srcRect.height
        //c_x = l_x/2.
        //c_y = l_y/2.
        //
        //D = sqrt( (c_x-l_x)**2 + (c_y-l_y)**2)
        //def f(x, y):
        //    xn = (x-c_x)/D
        //    yn = (y-c_y)/D
        //    r_d = sqrt( xn**2 + yn**2 )
        //    S = (1 + k1 * r_d**2)
        //    return xn*S*D+c_x, yn*S*D+c_y
        //
        double getPixelSize(const DPoint & pt); //A(x,y) -> A'(x',y')
    private:
        DPoint trapezoid(const DPoint &pt);
        DPoint distortion(const DPoint &pt);
        DPoint inv_trapezoid(const DPoint &pt);
        DPoint inv_distortion(const DPoint &pt);
        double m_TrapezoidFactor;
        //pincushion/barrel correction
        //K1<0 correct barrel
        //K1>0 correct pincushion
        //see http://www.imatest.com/docs/distortion.html
        DPoint m_Center;
        double m_K1;
        double m_RescaleFactor;
        double m_Scale;
        double m_TrapezoidScale;
        //DPoint **m_pCache; // m_pCache[x][y] - (new_x, new_y)
};

typedef boost::shared_ptr<CoordTransformer> CoordTransformerPtr;
}
#endif
