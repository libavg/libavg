//
// $Id$
//

#include "Timeout.h"

#include "../base/TimeSource.h"
#include "../base/Exception.h"

#include <iostream>

using namespace std;

namespace avg {

int Timeout::s_LastID = 0;

Timeout::Timeout(int time, PyObject * pyfunc, bool isInterval)
    : m_Interval(time),
      m_PyFunc(pyfunc),
      m_IsInterval(isInterval)
{
    m_NextTimeout = m_Interval+TimeSource::get()->getCurrentTicks();
//    cerr << "New timeout. m_Interval=" << m_Interval << ", m_NextTimeout="
//            << m_NextTimeout << endl;
    s_LastID++;
    m_ID = s_LastID;

    Py_INCREF(m_PyFunc);
}

Timeout::~Timeout()
{
    Py_DECREF(m_PyFunc);
}

bool Timeout::IsReady() const
{
    return m_NextTimeout <= TimeSource::get()->getCurrentTicks();
}

bool Timeout::IsInterval() const
{
    return m_IsInterval;
}

void Timeout::Fire()
{
    PyObject * arglist = Py_BuildValue("()");
    PyObject * result = PyEval_CallObject(m_PyFunc, arglist);
    if (!result) {
        cerr << "Error in timeout:" << endl;
        PyErr_Print();
        exit(-1);
        // TODO: The python function terminated with an exception.
    }
    Py_DECREF(arglist);    
    if (m_IsInterval) {
        m_NextTimeout = m_Interval + TimeSource::get()->getCurrentTicks();
//        cerr << "Interval::Fire. m_Interval=" << m_Interval << ", m_NextTimeout="
//            << m_NextTimeout << endl;
    }
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
