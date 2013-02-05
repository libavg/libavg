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

#include "PythonLogHandler.h"
#include "../base/Logger.h"

#include <boost/algorithm/string.hpp>

namespace avg
{
    

PythonLogHandler::PythonLogHandler(PyObject *pyLogger):
    m_pyLogger(pyLogger)
{
    Py_INCREF(pyLogger);
    assert(pyLogger);
}

PythonLogHandler::~PythonLogHandler()
{
    Py_DecRef(m_pyLogger);
}

void PythonLogHandler::logMessage(const tm* pTime, unsigned millis,
        const string& category, unsigned level, const UTF8String& sMsg)
{
    string sLevel = boost::to_lower_copy(string(Logger::levelToString(level)));
    PyObject * extra = PyDict_New();
    PyObject * pyCat = PyString_FromString(category.c_str());

    PyDict_SetItemString(extra, "category", pyCat);

    PyObject * pyMsg = PyString_FromString(sMsg.c_str());
    PyObject * args = PyTuple_New(1);
    PyObject * kwargs = PyDict_New();
    PyDict_SetItemString(kwargs, "extra", extra);
    PyTuple_SetItem(args, 0, pyMsg);

    PyObject_Call(PyObject_GetAttrString(m_pyLogger, sLevel.c_str()), args, kwargs);

    Py_DECREF(extra);
    Py_DECREF(pyCat);
    Py_DECREF(pyMsg);
    Py_DECREF(args);
    Py_DECREF(kwargs);
}

}
