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
    //delete m_pyLogger;
}

void PythonLogHandler::logMessage(const tm* pTime, unsigned millis,
        const string& category, long level, const UTF8String& sMsg)
{
    PyEval_CallMethod(m_pyLogger, boost::to_lower_copy(string(logging::levelToString(level))).c_str(),
            "(s)", sMsg.c_str());
}

}
