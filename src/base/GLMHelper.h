//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2014 Ulrich von Zadow
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

#ifndef _GLMHelper_H_
#define _GLMHelper_H_

#include "../api.h"

#define GLM_FORCE_RADIANS
#include "../glm/fwd.hpp"
#include "../glm/gtc/matrix_transform.hpp"
#include "../glm/gtc/type_ptr.hpp"
#include "../glm/gtx/io.hpp"

#include <iostream>
#include <vector>

namespace avg {

glm::vec2 getRotated(const glm::vec2& vec, float angle);
glm::vec2 getRotatedPivot(const glm::vec2& vec, float angle, 
        const glm::vec2& pivot=glm::vec2(0,0));
float getAngle(const glm::vec2& vec);
glm::vec2 fromPolar(float angle, float radius);

template<typename NUM, glm::precision precision>
bool almostEqual(const glm::detail::tvec2<NUM, precision>& v1,
                 const glm::detail::tvec2<NUM, precision>& v2);
template<typename NUM, glm::precision precision>
bool almostEqual(const glm::detail::tvec4<NUM, precision>& v1,
                 const glm::detail::tvec4<NUM, precision>& v2);

template<typename NUM, glm::precision precision>
std::istream& operator>>(std::istream& is, glm::detail::tvec2<NUM, precision>& p);
template<typename NUM, glm::precision precision>
std::istream& operator>>(std::istream& is, glm::detail::tvec3<NUM, precision>& p);

typedef glm::ivec2 IntPoint;
typedef std::vector<glm::vec2> Vec2Vector;

glm::vec2 stringToVec2(const std::string& s);
glm::vec3 stringToVec3(const std::string& s);
glm::ivec3 stringToIVec3(const std::string& s);

}

#endif
