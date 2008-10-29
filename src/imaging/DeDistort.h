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

#ifndef _DeDistort_H_
#define _DeDistort_H_

#include "../api.h"
#include "CoordTransformer.h"

#include "../base/Point.h"
#include "../base/Rect.h"

#include <boost/shared_ptr.hpp>

#include <libxml/parser.h>
#include <libxml/xmlwriter.h>

#include <vector>
#include <string>

namespace avg {

class TrackerConfig;

class AVG_API DeDistort: public CoordTransformer {
    public:
        DeDistort();
        DeDistort(const DPoint& CamExtents, const DPoint& DisplayExtents);
        DeDistort(const DPoint &CameraExtents,
            const std::vector<double>& DistortionParams, 
            double Angle, double TrapezoidFactor,
            const DPoint& DisplayOffset, const DPoint& DisplayScale);
        
        virtual ~DeDistort();

        DPoint transformBlobToScreen(const DPoint &pt);
        DPoint transformScreenToBlob(const DPoint &pt);
        virtual DPoint transform_point(const DPoint & pt); 
        virtual DPoint inverse_transform_point(const DPoint & pt);
        DRect getActiveBlobArea(const DPoint& DisplayExtents);

        void load(const DPoint &CameraExtents, const TrackerConfig& Config);
        void save(TrackerConfig& Config);

        bool operator ==(const DeDistort& other) const;

        void dump() const;

    private:
        double calc_rescale();
        DPoint inverse_undistort(const std::vector<double> &params, const DPoint &pt);
        DPoint undistort(const std::vector<double> &params, const DPoint &pt);
        DPoint trapezoid(const double trapezoid_factor, const DPoint &pt);
        DPoint inv_trapezoid(const double trapezoid_factor, const DPoint &pt);

        DPoint m_CamExtents;
        std::vector<double> m_DistortionParams;
        double m_Angle;
        double m_TrapezoidFactor;
        DPoint m_DisplayOffset;
        DPoint m_DisplayScale;
        
        double m_RescaleFactor;
};

typedef boost::shared_ptr<DeDistort> DeDistortPtr;

}
#endif
