
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

#include "SubscriberInfo.h"

#include "../base/Exception.h"
#include "../base/ObjectCounter.h"

#include <boost/python/slice.hpp>

using namespace std;

namespace avg {

py::object SubscriberInfo::s_MethodrefModule;

SubscriberInfo::SubscriberInfo(int id, const py::object& callable)
    : m_ID(id)
{
    ObjectCounter::get()->incRef(&typeid(*this));
    if (s_MethodrefModule.ptr() == py::object().ptr()) {
        s_MethodrefModule = py::import("libavg.methodref");
    }
    // Use the methodref module to manage the lifetime of the callables. This makes 
    // sure that we can delete bound-method callbacks when the object they are bound
    // to disappears.
    m_Callable = py::object(s_MethodrefModule.attr("methodref")(callable));
}

SubscriberInfo::~SubscriberInfo()
{
    ObjectCounter::get()->decRef(&typeid(*this));
}

bool SubscriberInfo::hasExpired() const
{
    py::object func = m_Callable();
    return (func.ptr() == py::object().ptr());
}

void SubscriberInfo::invoke(py::list args) const
{
    py::object callWeakRef = s_MethodrefModule.attr("callWeakRef");
    py::list argsCopy(args[py::slice()]);
    argsCopy.insert(0, m_Callable);
    py::tuple argsTuple(argsCopy);
    PyObject * pResult = PyObject_CallObject(callWeakRef.ptr(), argsTuple.ptr());
    if (pResult == 0) {
        throw py::error_already_set();
    }
}

int SubscriberInfo::getID() const
{
    return m_ID;
}
    
bool SubscriberInfo::isCallable(const py::object& callable) const
{
    bool bResult = py::call_method<bool>(m_Callable.ptr(), "isSameFunc", callable);
    return bResult;
}

}
