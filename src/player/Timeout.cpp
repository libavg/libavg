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

#include "Timeout.h"

#include "../base/Exception.h"
#include "../base/ObjectCounter.h"

#include "BoostPython.h"

#include <iostream>

using namespace boost::python;
using namespace std;

namespace avg {

int Timeout::s_LastID = 0;

Timeout::Timeout(int time, PyObject * pyfunc, bool isInterval, long long startTime)
    : m_Interval(time),
      m_PyFunc(pyfunc),
      m_IsInterval(isInterval)
{
    ObjectCounter::get()->incRef(&typeid(*this));
    m_NextTimeout = m_Interval+startTime;
    s_LastID++;
    m_ID = s_LastID;

    Py_INCREF(m_PyFunc);
}

Timeout::~Timeout()
{
    Py_DECREF(m_PyFunc);
    ObjectCounter::get()->decRef(&typeid(*this));
}

bool Timeout::IsReady(long long time) const
{
    return m_NextTimeout <= time;
}

bool Timeout::IsInterval() const
{
    return m_IsInterval;
}

void Timeout::Fire(long long curTime)
{
    if (m_IsInterval) {
        m_NextTimeout = m_Interval + curTime;
    }
    PyObject * arglist = Py_BuildValue("()");
    PyObject * result = PyEval_CallObject(m_PyFunc, arglist);
    // XXX: After the call to python, this might have been deleted 
    // by a call to clearTimeout()!
    Py_DECREF(arglist);    
    if (!result) {
        throw error_already_set();
    }
    Py_DECREF(result);
}

int Timeout::GetID() const
{
    return m_ID;
}

bool Timeout::operator <(const Timeout& other) const
{
    return m_NextTimeout < other.m_NextTimeout;
}

}
