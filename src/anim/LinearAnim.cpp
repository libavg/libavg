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

#include "LinearAnim.h"

#include "../player/Player.h"

using namespace boost::python;
using namespace std;

namespace avg {

LinearAnim::LinearAnim(const object& node, const string& sAttrName, long long duration,
            const object& startValue, const object& endValue, bool bUseInt, 
            const object& startCallback, const object& stopCallback)
    : SimpleAnim(node, sAttrName, duration, startValue, endValue, bUseInt, startCallback,
            stopCallback)
{
}

LinearAnim::~LinearAnim()
{
}

AnimPtr LinearAnim::create(const object& node, const string& sAttrName, long long duration,
        const object& pStartValue, const object& pEndValue, bool bUseInt, 
        const object& startCallback, const object& stopCallback)
{
    AnimPtr pAnim = AnimPtr(new LinearAnim(node, sAttrName, duration, pStartValue, 
            pEndValue, bUseInt, startCallback, stopCallback));
    return pAnim;
}

double LinearAnim::interpolate(double t)
{
    return t;
}

AnimPtr fadeIn(const boost::python::object& node, long long duration, double max, 
        const boost::python::object& stopCallback)
{
    object startVal = node.attr("opacity");
    AnimPtr pAnim = LinearAnim::create(node, "opacity", duration, startVal, 
            object(max), object(), stopCallback);
    pAnim->start(false);
    return pAnim;
}

AnimPtr fadeOut(const boost::python::object& node, long long duration, 
        const boost::python::object& stopCallback)
{
    object startVal = node.attr("opacity");
    AnimPtr pAnim = LinearAnim::create(node, "opacity", duration, startVal, 
            object(0), object(), stopCallback);
    pAnim->start(true);
    return pAnim;
}


    
}
