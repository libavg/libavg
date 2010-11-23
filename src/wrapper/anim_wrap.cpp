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

// All the XxxAnim_createN functions are there so we can make python constructors from
// the overloaded XxxAnim::create() functions in C++. These create functions return smart 
// pointers, so reference counting works correctly.
AnimPtr LinearAnim_create5(const object& node, const string& sAttrName, 
            long long duration, const object& startValue, const object& endValue)
{
    return LinearAnim::create(node, sAttrName, duration, startValue, endValue);
}

AnimPtr LinearAnim_create6(const object& node, const string& sAttrName, 
            long long duration, const object& startValue, const object& endValue, 
            bool bUseInt)
{
    return LinearAnim::create(node, sAttrName, duration, startValue, endValue, bUseInt);
}

AnimPtr LinearAnim_create7(const object& node, const string& sAttrName, 
            long long duration, const object& startValue, const object& endValue, 
            bool bUseInt, const object& startCallback)
{
    return LinearAnim::create(node, sAttrName, duration, startValue, endValue, bUseInt, 
            startCallback);
}

AnimPtr EaseInOutAnim_create7(const object& node, const string& sAttrName, 
            long long duration, const object& startValue, const object& endValue, 
            long long easeInDuration, long long easeOutDuration)
{
    return EaseInOutAnim::create(node, sAttrName, duration, startValue, endValue, 
            easeInDuration, easeOutDuration);
}

AnimPtr EaseInOutAnim_create8(const object& node, const string& sAttrName, 
            long long duration, const object& startValue, const object& endValue, 
            long long easeInDuration, long long easeOutDuration, bool bUseInt)
{
    return EaseInOutAnim::create(node, sAttrName, duration, startValue, endValue, 
            easeInDuration, easeOutDuration, bUseInt);
}

AnimPtr EaseInOutAnim_create9(const object& node, const string& sAttrName, 
            long long duration, const object& startValue, const object& endValue, 
            long long easeInDuration, long long easeOutDuration, bool bUseInt, 
            const object& startCallback)
{
    return EaseInOutAnim::create(node, sAttrName, duration, startValue, endValue, 
            easeInDuration, easeOutDuration, bUseInt, startCallback);
}

AnimPtr ContinuousAnim_create4(const object& node, const string& sAttrName,
            const object& startValue, const object& speed)
{
    return ContinuousAnim::create(node, sAttrName, startValue, speed);
}

AnimPtr ContinuousAnim_create5(const object& node, const string& sAttrName,
            const object& startValue, const object& speed, bool bUseInt)
{
    return ContinuousAnim::create(node, sAttrName, startValue, speed, bUseInt);
}

AnimPtr ContinuousAnim_create6(const object& node, const string& sAttrName,
            const object& startValue, const object& speed, bool bUseInt, 
            const object& startCallback)
{
    return ContinuousAnim::create(node, sAttrName, startValue, speed, bUseInt,
            startCallback);
}

AnimPtr ContinuousAnim_create7(const object& node, const string& sAttrName,
            const object& startValue, const object& speed, bool bUseInt,
            const object& startCallback, const object& stopCallback)
{
    return ContinuousAnim::create(node, sAttrName, startValue, speed, bUseInt,
            startCallback, stopCallback);
}

AnimPtr WaitAnim_create0()
{
    return WaitAnim::create();
}

AnimPtr WaitAnim_create1(long long duration)
{
    return WaitAnim::create(duration);
}

AnimPtr WaitAnim_create2(long long duration, const object& startCallback)
{
    return WaitAnim::create(duration, startCallback);
}

AnimPtr ParallelAnim_create1(const vector<AnimPtr>& anims)
{
    return ParallelAnim::create(anims);
}

AnimPtr ParallelAnim_create2(const vector<AnimPtr>& anims, const object& startCallback)
{
    return ParallelAnim::create(anims, startCallback);
}

AnimPtr ParallelAnim_create3(const vector<AnimPtr>& anims,
            const object& startCallback, const object& stopCallback)
{
    return ParallelAnim::create(anims, startCallback, stopCallback);
}

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
            boost::noncopyable>("LinearAnim", no_init)
        .def("__init__", make_constructor(LinearAnim_create5))
        .def("__init__", make_constructor(LinearAnim_create6))
        .def("__init__", make_constructor(LinearAnim_create7))
        .def("__init__", make_constructor(LinearAnim::create)) 
        ;
   
    class_<EaseInOutAnim, boost::shared_ptr<EaseInOutAnim>, bases<SimpleAnim>, 
            boost::noncopyable>("EaseInOutAnim", no_init)
        .def("__init__", make_constructor(EaseInOutAnim_create7))
        .def("__init__", make_constructor(EaseInOutAnim_create8))
        .def("__init__", make_constructor(EaseInOutAnim_create9))
        .def("__init__", make_constructor(EaseInOutAnim::create))
        ;
   
    class_<ContinuousAnim, boost::shared_ptr<ContinuousAnim>, bases<AttrAnim>, 
            boost::noncopyable>("ContinuousAnim", no_init)
        .def("__init__", make_constructor(ContinuousAnim_create4)) 
        .def("__init__", make_constructor(ContinuousAnim_create5)) 
        .def("__init__", make_constructor(ContinuousAnim_create6)) 
        .def("__init__", make_constructor(ContinuousAnim_create7)) 
        ;

    class_<WaitAnim, boost::shared_ptr<WaitAnim>, bases<Anim>, boost::noncopyable>(
            "WaitAnim", no_init)
        .def("__init__", make_constructor(WaitAnim_create0))
        .def("__init__", make_constructor(WaitAnim_create1))
        .def("__init__", make_constructor(WaitAnim_create2))
        .def("__init__", make_constructor(WaitAnim::create))
        .def("start", &WaitAnim::start, start_overloads(args("bKeepAttr")))
        ;
    
    class_<ParallelAnim, boost::shared_ptr<ParallelAnim>, bases<Anim>, 
            boost::noncopyable>("ParallelAnim", no_init)
        .def("__init__", make_constructor(ParallelAnim_create1))
        .def("__init__", make_constructor(ParallelAnim_create2))
        .def("__init__", make_constructor(ParallelAnim_create3))
        .def("__init__", make_constructor(ParallelAnim::create))
        .def("start", &ParallelAnim::start, start_overloads(args("bKeepAttr")))
        ;
      
    class_<AnimState, boost::noncopyable>("AnimState", no_init)
        .def(init<const string&, AnimPtr>())
        .def(init<const string&, AnimPtr, const string& >())
        ;

    class_<StateAnim, boost::shared_ptr<StateAnim>, bases<Anim>, 
            boost::noncopyable>("StateAnim", no_init)
        .def("__init__", make_constructor(StateAnim::create))
        .def("setState", &StateAnim::setState, setState_overloads(args("bKeepAttr")))
        .def("getState", make_function(&StateAnim::getState,
                return_value_policy<copy_const_reference>()))
        .def("setDebug", &StateAnim::setDebug) 
        ;

    def("fadeIn", fadeIn, fadeIn_overloads(args("max", "stopCallback")));

    def("fadeOut", fadeOut, fadeOut_overloads(args("stopCallback")));
}
