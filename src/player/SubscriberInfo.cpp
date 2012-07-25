
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

using namespace std;

namespace avg {

boost::python::object SubscriberInfo::s_MethodrefModule;

SubscriberInfo::SubscriberInfo(int id, const boost::python::object& callable)
    : m_ID(id)
{
    if (s_MethodrefModule.ptr() == boost::python::object().ptr()) {
        s_MethodrefModule = boost::python::import("libavg.methodref");
    }
    // Use the methodref module to manage the lifetime of the callables. This makes 
    // sure that all callbacks are deleted when the publisher disappears.
    m_Callable = boost::python::object(s_MethodrefModule.attr("methodref")(callable));
}

SubscriberInfo::~SubscriberInfo()
{
}

bool SubscriberInfo::hasExpired() const
{
    boost::python::object func = m_Callable();
    return (func.ptr() == boost::python::object().ptr());
}

void SubscriberInfo::invoke(boost::python::list args) const
{
    boost::python::object callWeakRef = s_MethodrefModule.attr("callWeakRef");
    args.insert(0, m_Callable);
    boost::python::tuple argsTuple(args);
    PyObject * pResult = PyObject_CallObject(callWeakRef.ptr(), argsTuple.ptr());
    if (pResult == 0) {
        throw boost::python::error_already_set();
    }
}

int SubscriberInfo::getID() const
{
    return m_ID;
}

}
