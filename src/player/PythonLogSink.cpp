//
//  libavg - Media Playback Engine.
//  Copyright (C) 2003-2012 Ulrich von Zadow
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

#include "PythonLogSink.h"
#include "WrapPython.h"
#include "../base/Logger.h"

#include <boost/algorithm/string.hpp>
#include <boost/python/errors.hpp>

namespace avg
{

PythonLogSink::PythonLogSink(PyObject *pyLogger):
    m_pyLogger(pyLogger)
{
    Py_INCREF(pyLogger);
    assert(pyLogger);
}

PythonLogSink::~PythonLogSink()
{
    Py_DecRef(m_pyLogger);
}

const char * PythonLogSink::LogSeverityToPythonString(severity_t severity)
{
    if(severity == Logger::severity::CRITICAL) {
        return "critical";
    } else if(severity == Logger::severity::ERROR) {
        return "error";
    } else if(severity == Logger::severity::WARNING) {
        return "warning";
    } else if(severity == Logger::severity::INFO) {
        return "info";
    } else if(severity == Logger::severity::DEBUG) {
        return "debug";
    }
    throw Exception(AVG_ERR_UNKNOWN, "Unkown log severity");
}

void PythonLogSink::logMessage(const tm* pTime, unsigned millis,
        const category_t& category, severity_t severity, const UTF8String& sMsg)
{
    try {
        aquirePyGIL aquireGil;
        PyObject * extra = PyDict_New();
        PyObject * pyCat = PyString_FromString(category.c_str());

        PyDict_SetItemString(extra, "category", pyCat);

        PyObject * pyMsg = PyString_FromString(sMsg.c_str());
        PyObject * args = PyTuple_New(1);
        PyObject * kwargs = PyDict_New();
        PyDict_SetItemString(kwargs, "extra", extra);
        PyTuple_SetItem(args, 0, pyMsg);

        PyObject_Call(PyObject_GetAttrString(m_pyLogger,
                LogSeverityToPythonString(severity)), args, kwargs);

        Py_DECREF(extra);
        Py_DECREF(pyCat);
        Py_DECREF(args);
        Py_DECREF(kwargs);
    } catch (const boost::python::error_already_set &err) {
        cerr << "PythonLogSink: Python raised exception\n";
    } catch (const exception &err) {
        cerr << "PythonLogSink: Couldn't log to python logger.\n";
    }
}

}
