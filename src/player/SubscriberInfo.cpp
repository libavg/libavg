
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

#include "SubscriberInfo.h"

#include "../base/Exception.h"
#include "../base/ObjectCounter.h"
#include "../base/ScopeTimer.h"

#include <boost/python/slice.hpp>


using namespace std;

namespace avg {

SubscriberInfo::SubscriberInfo(int id, PyObject* pCallable)
    : m_ID(id),
      m_pWeakSelf(Py_None),
      m_pPyFunction(Py_None),
      m_pWeakClass(Py_None)
{
    ObjectCounter::get()->incRef(&typeid(*this));
    AVG_ASSERT(PyCallable_Check(pCallable));

    if(PyMethod_Check(pCallable)) { //Bound method
        m_pWeakSelf = PyWeakref_NewRef(PyMethod_Self(pCallable), NULL);
        AVG_ASSERT(m_pWeakSelf != Py_None);
        m_pPyFunction = PyWeakref_NewRef(PyMethod_Function(pCallable), NULL);
        AVG_ASSERT(m_pPyFunction != Py_None);
#if PY_MAJOR_VERSION < 3
        m_pWeakClass = PyWeakref_NewRef(PyMethod_Class(pCallable), NULL);
#endif

    }else{
        m_pPyFunction = pCallable;
        Py_INCREF(m_pPyFunction); //We need to keep a reference to the unbound function
        AVG_ASSERT(m_pPyFunction != Py_None);
    }
}

SubscriberInfo::~SubscriberInfo()
{
    ObjectCounter::get()->decRef(&typeid(*this));
    if(m_pWeakSelf != Py_None){
        Py_DECREF(m_pWeakSelf);
    }
    Py_DECREF(m_pPyFunction);
}

bool SubscriberInfo::hasExpired() const
{
    bool expired;
    if(m_pWeakSelf == Py_None) { //Unbound function
        return false; //Unbound functions can't really expire
    }else{
        //If self still exists, the function should still be valid or people are messing
        //too deeply with python
        expired = PyWeakref_GetObject(m_pWeakSelf) == Py_None;
    }
    return expired;
}

static ProfilingZoneID InvokeSubscriberProfilingZone("SubscriberInfo: invoke");

void SubscriberInfo::invoke(py::list args) const
{
    ScopeTimer timer(InvokeSubscriberProfilingZone);

    if(m_pWeakSelf != Py_None) { //Bound method case
        PyObject * pFunction = PyWeakref_GetObject(m_pPyFunction);
        AVG_ASSERT(pFunction != Py_None);
        PyObject * pSelf = PyWeakref_GetObject(m_pWeakSelf);
        AVG_ASSERT(pSelf != Py_None);
#if PY_MAJOR_VERSION < 3
        PyObject * pClass = PyWeakref_GetObject(m_pWeakClass);
        AVG_ASSERT(pClass != Py_None);
        PyObject * pCallable = PyMethod_New(pFunction, pSelf, pClass);  //Bind function to self --> creating a bound method
#else
        PyObject * pCallable = PyMethod_New(pFunction, pSelf);  //Bind function to self --> creating a bound method
#endif
        AVG_ASSERT(pCallable != Py_None);
        py::tuple argsTuple(args);
        PyObject* pyResult = PyObject_CallObject(pCallable, argsTuple.ptr());
        if (pyResult == NULL) {
            throw py::error_already_set();
        }
        Py_DECREF(pCallable);
        Py_DECREF(pyResult);
    }else{ //unbound method case
        py::tuple argsTuple(args);
        PyObject* pyResult = PyObject_CallObject(m_pPyFunction, argsTuple.ptr());
        if (pyResult == NULL) {
            throw py::error_already_set();
        }
        Py_XDECREF(pyResult);
    }
}

int SubscriberInfo::getID() const
{
    return m_ID;
}

bool SubscriberInfo::isCallable(const PyObject* pCallable) const
{
    if(m_pWeakSelf != Py_None) {
        PyObject * lhsCallable = PyWeakref_GetObject(m_pPyFunction);
        PyObject * rhsCallable = PyMethod_Function(const_cast<PyObject*>(pCallable));
        return lhsCallable == rhsCallable;
    }else{
        return m_pPyFunction == pCallable;
    }
}

}
