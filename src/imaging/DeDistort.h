//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2011 Ulrich von Zadow
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

#include "../base/GLMHelper.h"
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
        DeDistort(const glm::vec2& camExtents, const glm::vec2& displayExtents);
        DeDistort(const glm::vec2& camExtents, 
                const std::vector<double>& distortionParams, double angle, 
                double trapezoidFactor, const glm::dvec2& displayOffset, 
                const glm::dvec2& displayScale);
        virtual ~DeDistort();

        glm::dvec2 transformBlobToScreen(const glm::dvec2& pt);
        glm::dvec2 transformScreenToBlob(const glm::dvec2& pt);
        virtual glm::dvec2 transform_point(const glm::dvec2& pt); 
        virtual glm::dvec2 inverse_transform_point(const glm::dvec2& pt);
        FRect getDisplayArea(const glm::vec2& displayExtents);
        FRect getActiveBlobArea(const FRect& displayROI);

        void load(const glm::vec2 &CameraExtents, const TrackerConfig& config);
        void save(TrackerConfig& config);

        bool operator ==(const DeDistort& other) const;

        void dump() const;

    private:
        double calc_rescale();
        glm::dvec2 inverse_undistort(const std::vector<double>& params,
                const glm::dvec2& pt);
        glm::dvec2 undistort(const std::vector<double>& params, const glm::dvec2& pt);
        glm::dvec2 trapezoid(const double trapezoid_factor, const glm::dvec2& pt);
        glm::dvec2 inv_trapezoid(const double trapezoid_factor, const glm::dvec2& pt);

        glm::dvec2 m_CamExtents;
        std::vector<double> m_DistortionParams;
        double m_Angle;
        double m_TrapezoidFactor;
        glm::dvec2 m_DisplayOffset;
        glm::dvec2 m_DisplayScale;
        
        double m_RescaleFactor;
};

typedef boost::shared_ptr<DeDistort> DeDistortPtr;

}
#endif
