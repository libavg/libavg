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

#include "LinearAnim.h"

#include "../player/Player.h"

using namespace std;

namespace avg {

LinearAnim::LinearAnim(const bp::object& node, const string& sAttrName, long long duration,
            const bp::object& startValue, const bp::object& endValue, bool bUseInt, 
            const bp::object& startCallback, const bp::object& stopCallback,
            const bp::object& abortCallback)
    : SimpleAnim(node, sAttrName, duration, startValue, endValue, bUseInt, startCallback,
            stopCallback, abortCallback)
{
}

LinearAnim::~LinearAnim()
{
}

float LinearAnim::interpolate(float t)
{
    return t;
}

float LinearAnim::getStartPart(float start, float end, float cur)
{
    return (cur-start)/(end-start);
}

AnimPtr fadeIn(const bp::object& node, long long duration, float max, 
        const bp::object& stopCallback)
{
    bp::object startVal = node.attr("opacity");
    AnimPtr pAnim(new LinearAnim(node, "opacity", duration, startVal, 
            bp::object(max), false, bp::object(), stopCallback));
    pAnim->start(false);
    return pAnim;
}

AnimPtr fadeOut(const bp::object& node, long long duration, 
        const bp::object& stopCallback)
{
    bp::object startVal = node.attr("opacity");
    AnimPtr pAnim(new LinearAnim(node, "opacity", duration, startVal, 
            bp::object(0), false, bp::object(), stopCallback));
    pAnim->start(true);
    return pAnim;
}

}
