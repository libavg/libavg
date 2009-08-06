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
#include "../anim/WaitAnim.h"
#include "../anim/GroupAnim.h"
#include "../anim/ParallelAnim.h"
#include "../anim/StateAnim.h"

#include "../player/BoostPython.h"

using namespace boost::python;
using namespace std;
using namespace avg;

BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(simple_start_overloads, SimpleAnim::start, 0, 1);
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(wait_start_overloads, WaitAnim::start, 0, 1);
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(parallel_start_overloads, ParallelAnim::start, 
        0, 1);
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(setState_overloads, StateAnim::setState, 1, 2);

void export_anim()
{
    from_python_sequence<vector<AnimPtr>, variable_capacity_policy>();
    from_python_sequence<vector<AnimState>, variable_capacity_policy>();
    
    def("getNumRunningAnims", SimpleAnim::getNumRunningAnims);
    
    class_<Anim, boost::noncopyable>("Anim", no_init)
        .def("setStartCallback", &Anim::setStartCallback)
        .def("setStopCallback", &Anim::setStopCallback)
        .def("abort", &Anim::abort, "Stops the animation.")
        .def("isRunning", &Anim::isRunning)
        ;

    class_<SimpleAnim, bases<Anim>, boost::noncopyable>("SimpleAnim",
            "Base class for animations that change libavg node attributes by\n"
            "interpolating over a set amount of time. Constructing an animation\n"
            "object starts the animation. If abort() isn't needed, there is no need\n"
            "to hold on to the object - it will exist exactly as long as the animation\n"
            "lasts and then disappear.\n\n"
            "The animation framework makes sure that only one animation per attribute\n"
            "of a node runs at any given time. If a second one is started, the first\n"
            "one is aborted.",
            no_init)
        .def("start", &SimpleAnim::start, simple_start_overloads(args("bKeepAttr")))
        ;

    class_<LinearAnim, bases<SimpleAnim>, boost::noncopyable>("LinearAnim",
            "Class that animates an attribute of a libavg node by interpolating\n"
            "linearly between start and end values.", no_init)
        .def(init<const object&, const string&, long long, const object&, 
                const object&>())
        .def(init<const object&, const string&, long long, const object&, const object&,
                bool>())
        .def(init<const object&, const string&, long long, const object&, const object&,
                bool, const object&>())
        .def(init<const object&, const string&, long long, const object&, const object&,
                bool, const object&, const object&>(
                "@param node: The libavg node object to animate.\n"
                "@param attrName: The name of the attribute to change. Must be a \n"
                "numeric attribute.\n"
                "@param duration: The length of the animation in milliseconds.\n"
                "@param startValue: Initial value of the attribute.\n"
                "@param endValue: Value of the attribute after duration has elapsed.\n"
                "@param useInt: If True, the attribute is always set to an integer\n"
                "value.\n"
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
        .def(init<const object&, const string&, long long, const object&, const object&,
                long long, long long>())
        .def(init<const object&, const string&, long long, const object&, const object&,
                long long, long long, bool>())
        .def(init<const object&, const string&, long long, const object&, const object&,
                long long, long long, bool, const object&>())
        .def(init<const object&, const string&, long long, const object&, const object&,
                long long, long long, bool, const object&, const object&>())
        ;
    
    class_<WaitAnim, bases<Anim>, boost::noncopyable>("WaitAnim", no_init)
        .def(init<>())
        .def(init<long long>())
        .def(init<long long, const object&>())
        .def(init<long long, const object&, const object&>())
        .def("start", &WaitAnim::start, wait_start_overloads(args("bKeepAttr")))
        ;
    
    class_<GroupAnim, bases<Anim>, boost::noncopyable>("GroupAnim", no_init)
        .def("childStopped", &GroupAnim::childStopped)
        ;

    class_<ParallelAnim, bases<GroupAnim>, boost::noncopyable>("ParallelAnim", no_init)
        .def(init<vector<AnimPtr> >())
        .def(init<vector<AnimPtr>, const object&>())
        .def(init<vector<AnimPtr>, const object&, const object&>())
        .def(init<vector<AnimPtr>, const object&, const object&, long long>())
        .def("start", &ParallelAnim::start, parallel_start_overloads(args("bKeepAttr")))
        ;
      
    class_<AnimState, boost::noncopyable>("AnimState", no_init)
        .def(init<const string&, AnimPtr>())
        .def(init<const string&, AnimPtr, const string& >())
        ;

    class_<StateAnim, bases<GroupAnim>, boost::noncopyable>("StateAnim", no_init)
        .def(init<vector<AnimState> >())
        .def("setState", &StateAnim::setState, setState_overloads(args("bKeepAttr")))
        .def("getState", make_function(&StateAnim::getState,
                return_value_policy<copy_const_reference>()), "")
        .def("setDebug", &StateAnim::setDebug, "")
        ;
}
