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

#ifndef _DeDistort_H_
#define _DeDistort_H_

#include "CoordTransformer.h"
#include "DistortionParams.h"

#include "../graphics/Point.h"
#include "../graphics/Rect.h"

#include <boost/shared_ptr.hpp>

#include <vector>

namespace avg {

class DeDistort: public CoordTransformer {
    public:
        DeDistort();
        DeDistort(const DistortionParams& Params);
        virtual ~DeDistort();

        void load(xmlNodePtr pRootNode);

        virtual DPoint transform_point(const DPoint & pt); //(x,y) -> (x', y')
        virtual DPoint inverse_transform_point(const DPoint & pt); //(x,y) -> (x', y')
    
    private:
        double calc_rescale();
        DPoint inverse_undistort(const std::vector<double> &params, const DPoint &pt) ;
        DPoint undistort(const std::vector<double> &params, const DPoint &pt) ;
        DPoint scale(const DPoint &scales, const DPoint &pt);
        DPoint scale(const double scale, const DPoint &pt);
        DPoint trapezoid(const double trapezoid_factor, const DPoint &pt);
        DPoint inv_trapezoid(const double trapezoid_factor, const DPoint &pt);
        DPoint translate(const DPoint &displacement, const DPoint &pt);
        DPoint rotate(double angle, const DPoint &pt);
        DPoint inverse_pinhole(const DPoint3 &P, const DPoint3 &N, const DPoint &pt);
        DPoint pinhole(const DPoint3& P, const DPoint3& N, const DPoint &pt);

        DistortionParams m_Params;
        double m_RescaleFactor;
};

}
#endif
