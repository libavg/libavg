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

#include "WrapHelper.h"

#include "../anim/SimpleAnim.h"
#include "../anim/LinearAnim.h"
#include "../anim/EaseInOutAnim.h"
#include "../player/BoostPython.h"

using namespace boost::python;
using namespace std;
using namespace avg;

BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(start_overloads, SimpleAnim::start, 0, 1);

void export_anim()
{
    def("getNumRunningAnims", SimpleAnim::getNumRunningAnims);

    class_<SimpleAnim, boost::noncopyable>("SimpleAnim",
            "Base class for animations that change libavg node attributes by\n"
            "interpolating over a set amount of time. Constructing an animation\n"
            "object starts the animation. If abort() isn't needed, there is no need\n"
            "to hold on to the object - it will exist exactly as long as the animation\n"
            "lasts and then disappear.\n\n"
            "The animation framework makes sure that only one animation per attribute\n"
            "of a node runs at any given time. If a second one is started, the first\n"
            "one is aborted.",
            no_init)
        .def("setStartCallback", &SimpleAnim::setStartCallback)
        .def("setStopCallback", &SimpleAnim::setStopCallback)
        .def("start", &SimpleAnim::start, start_overloads(args("bKeepAttr")))
        .def("abort", &SimpleAnim::abort, 
                "Stops the animation. Does not call onStop()")
        .def("isRunning", &SimpleAnim::isRunning)
        ;

    class_<LinearAnim, bases<SimpleAnim>, boost::noncopyable>("LinearAnim",
            "Class that animates an attribute of a libavg node by interpolating linearly\n"
            "between start and end values.", no_init)
        .def(init<const object&, const string&, double, const object&, const object&,
                bool>())
        .def(init<const object&, const string&, double, const object&, const object&,
                bool, const object&>())
        .def(init<const object&, const string&, double, const object&, const object&,
                bool, const object&, const object&>(
                "@param node: The libavg node object to animate.\n"
                "@param attrName: The name of the attribute to change. Must be a numeric\n"
                "attribute.\n"
                "@param duration: The length of the animation in milliseconds.\n"
                "@param startValue: Initial value of the attribute.\n"
                "@param endValue: Value of the attribute after duration has elapsed.\n"
                "@param useInt: If True, the attribute is always set to an integer value.\n"
                "@param onStop: Python callable to invoke when duration has elapsed and\n"
                "the animation has finished. This can be used to chain\n"
                "animations together by using lambda to create a second animation.\n"))
        ;
    
    class_<EaseInOutAnim, bases<SimpleAnim>, boost::noncopyable>("EaseInOutAnim",
            "Class that animates an attribute of a libavg node. The animation proceeds\n"
            "in three phases: ease-in, linear and ease-out. Start and end speed are\n"
            "zero. Ease-in and ease-out phases have the shape of one quadrant of the\n"
            "sine curve.",
            no_init)
        .def(init<const object&, const string&, double, const object&, const object&,
                double, double, bool>())
        .def(init<const object&, const string&, double, const object&, const object&,
                double, double, bool, const object&>())
        .def(init<const object&, const string&, double, const object&, const object&,
                double, double, bool, const object&, const object&>())
        ;
}
