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

#include "GLMHelper.h"
#include "StringHelper.h"
#include "MathHelper.h"

#include "../glm/gtx/rotate_vector.hpp"

using namespace std;

namespace avg {

glm::vec2 getRotated(const glm::vec2& vec, float angle)
{
    return glm::rotate(vec, angle);
}

glm::vec2 getRotatedPivot(const glm::vec2& vec, float angle, const glm::vec2& pivot)
{
    // translate pivot to origin
    glm::vec2 translated = vec - pivot;
   
    // calculate rotated coordinates about the origin
    glm::vec2 rotated = glm::rotate(translated, angle);

    // re-translate pivot to original position
    rotated += pivot;

    return rotated;
}

float getAngle(const glm::vec2& vec)
{
    return float(atan2(double(vec.y), double(vec.x)));
}

glm::vec2 fromPolar(float angle, float radius)
{
    return glm::vec2(cos(angle)*radius, sin(angle)*radius);
}

template<typename NUM, glm::precision precision>
bool almostEqual(const glm::detail::tvec2<NUM, precision>& v1,
                 const glm::detail::tvec2<NUM, precision>& v2)
{
    return (fabs(v1.x-v2.x)+fabs(v1.y-v2.y)) < 0.0001;
}

template<typename NUM, glm::precision precision>
bool almostEqual(const glm::detail::tvec4<NUM, precision>& v1,
                 const glm::detail::tvec4<NUM, precision>& v2)
{
    return (fabs(v1.x-v2.x)+fabs(v1.y-v2.y)+fabs(v1.z-v2.z)+fabs(v1.w-v2.w)) < 0.0001;
}

template<typename NUM, glm::precision precision>
std::istream& operator>>(std::istream& is, glm::detail::tvec2<NUM, precision>& p)
{
    skipToken(is, '(');
    is >> p.x;
    skipToken(is, ',');
    is >> p.y;
    skipToken(is, ')');
    return is;
}

template<typename NUM, glm::precision precision>
std::istream& operator>>(std::istream& is, glm::detail::tvec3<NUM, precision>& p)
{
    skipToken(is, '(');
    is >> p.x;
    skipToken(is, ',');
    is >> p.y;
    skipToken(is, ',');
    is >> p.z;
    skipToken(is, ')');
    return is;
}

glm::vec2 stringToVec2(const std::string& s)
{
    glm::vec2 pt;
    fromString(s, pt);
    return pt;
}

glm::vec3 stringToVec3(const std::string& s)
{
    glm::vec3 pt;
    fromString(s, pt);
    return pt;
}

glm::ivec3 stringToIVec3(const std::string& s)
{
    glm::ivec3 pt;
    fromString(s, pt);
    return pt;
}


template AVG_TEMPLATE_API std::istream& operator>>(std::istream& is, glm::detail::tvec2<int, glm::highp>& p);
template AVG_TEMPLATE_API std::istream& operator>>(std::istream& is, glm::detail::tvec2<float, glm::highp>& p);
template AVG_TEMPLATE_API std::istream& operator>>(std::istream& is, glm::detail::tvec2<double, glm::highp>& p);

template AVG_TEMPLATE_API std::istream& operator>>(std::istream& is, glm::detail::tvec3<int, glm::highp>& p);
template AVG_TEMPLATE_API std::istream& operator>>(std::istream& is, glm::detail::tvec3<float, glm::highp>& p);
template AVG_TEMPLATE_API std::istream& operator>>(std::istream& is, glm::detail::tvec3<double, glm::highp>& p);

template AVG_TEMPLATE_API bool almostEqual(const glm::detail::tvec2<float, glm::highp>& v1,
        const glm::detail::tvec2<float, glm::highp>& v2);
template AVG_TEMPLATE_API bool almostEqual(const glm::detail::tvec2<double, glm::highp>& v1,
        const glm::detail::tvec2<double, glm::highp>& v2);
template AVG_TEMPLATE_API bool almostEqual(const glm::detail::tvec4<float, glm::highp>& v1,
        const glm::detail::tvec4<float, glm::highp>& v2);
template AVG_TEMPLATE_API bool almostEqual(const glm::detail::tvec4<double, glm::highp>& v1,
        const glm::detail::tvec4<double, glm::highp>& v2);
}

