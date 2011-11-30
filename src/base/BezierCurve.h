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

#ifndef _BezierCurve_H_
#define _BezierCurve_H_

#include "../api.h"

#include "../glm/glm.hpp"

#include <boost/shared_ptr.hpp>
#include <vector>

namespace avg {

class AVG_API BezierCurve {
public:
    BezierCurve(const glm::vec2& p0, const glm::vec2& p1, const glm::vec2& p2,
            const glm::vec2& p3);

    glm::vec2 interpolate(float t) const;
    glm::vec2 getDeriv(float t) const;

private:
    glm::vec2 m_P0;
    glm::vec2 m_P1;
    glm::vec2 m_P2;
    glm::vec2 m_P3;
};

typedef boost::shared_ptr<BezierCurve> BezierCurvePtr;

}

#endif 



