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

#include "WrapHelper.h"

#include "../anim/SimpleAnim.h"
#include "../anim/LinearAnim.h"
#include "../anim/EaseInOutAnim.h"
#include "../anim/ContinuousAnim.h"
#include "../anim/WaitAnim.h"
#include "../anim/ParallelAnim.h"
#include "../anim/StateAnim.h"

#include "../player/BoostPython.h"

using namespace boost::python;
using namespace std;
using namespace avg;

BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(start_overloads, start, 0, 1);
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(setState_overloads, StateAnim::setState, 1, 2);
BOOST_PYTHON_FUNCTION_OVERLOADS(fadeIn_overloads, fadeIn, 2, 4);
BOOST_PYTHON_FUNCTION_OVERLOADS(fadeOut_overloads, fadeOut, 2, 3);

void export_anim()
{
    from_python_sequence<vector<AnimPtr>, variable_capacity_policy>();
    from_python_sequence<vector<AnimState>, variable_capacity_policy>();
    
    def("getNumRunningAnims", AttrAnim::getNumRunningAnims);
    
    class_<Anim, boost::shared_ptr<Anim>, boost::noncopyable>("Anim", no_init)
        .def("setStartCallback", &Anim::setStartCallback)
        .def("setStopCallback", &Anim::setStopCallback)
        .def("abort", &Anim::abort)
        .def("isRunning", &Anim::isRunning)
        ;

    class_<AttrAnim, boost::shared_ptr<AttrAnim>, bases<Anim>, boost::noncopyable>
            ("AttrAnim", no_init)
        .def("start", &AttrAnim::start, start_overloads(args("bKeepAttr")))
        ;
 
    class_<SimpleAnim, boost::shared_ptr<SimpleAnim>, bases<AttrAnim>, 
            boost::noncopyable>("SimpleAnim", no_init)
        ;

    class_<LinearAnim, boost::shared_ptr<LinearAnim>, bases<SimpleAnim>, 
            boost::noncopyable>
            ("LinearAnim", init<const object&, const string&, long long, const object&, 
                    const object&, optional<bool, const object&, const object&> >())
        ;
   
    class_<EaseInOutAnim, boost::shared_ptr<EaseInOutAnim>, bases<SimpleAnim>, 
            boost::noncopyable>
            ("EaseInOutAnim", init<const object&, const string&, long long, const object&,
                    const object&, long long, long long, 
                    optional<bool, const object&, const object&> >())
        ;
   
    class_<ContinuousAnim, boost::shared_ptr<ContinuousAnim>, bases<AttrAnim>, 
            boost::noncopyable>(
                    "ContinuousAnim", init<const object&, const string&,
                    const object&, const object&, 
                    optional<bool, const object&, const object&> >())
        ;

    class_<WaitAnim, boost::shared_ptr<WaitAnim>, bases<Anim>, boost::noncopyable>(
            "WaitAnim", init<optional<long long, const object&, const object&> >())
        .def("start", &WaitAnim::start, start_overloads(args("bKeepAttr")))
        ;
    
    class_<ParallelAnim, boost::shared_ptr<ParallelAnim>, bases<Anim>, 
            boost::noncopyable>("ParallelAnim", init<const vector<AnimPtr>&,
                    optional<const object&, const object&, long long> >())
        .def("start", &ParallelAnim::start, start_overloads(args("bKeepAttr")))
        ;
      
    class_<AnimState, boost::noncopyable>("AnimState",
            init<const string&, AnimPtr, optional<const string&> >())
        ;

    class_<StateAnim, boost::shared_ptr<StateAnim>, bases<Anim>, 
            boost::noncopyable>("StateAnim", init<const vector<AnimState>&>())
        .def("setState", &StateAnim::setState, setState_overloads(args("bKeepAttr")))
        .def("getState", make_function(&StateAnim::getState,
                return_value_policy<copy_const_reference>()))
        .def("setDebug", &StateAnim::setDebug) 
        ;

    def("fadeIn", fadeIn, fadeIn_overloads(args("max", "stopCallback")));

    def("fadeOut", fadeOut, fadeOut_overloads(args("stopCallback")));
}
