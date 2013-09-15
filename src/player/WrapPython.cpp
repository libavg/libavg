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

#include "WrapPython.h"

#include "../base/Logger.h"
#include "../base/StringHelper.h"
#include "../base/FileHelper.h"

#include <frameobject.h>

using namespace std;

namespace avg {

aquirePyGIL::aquirePyGIL()
{
    m_pyGilState = PyGILState_Ensure();
}
aquirePyGIL::~aquirePyGIL()
{
    PyGILState_Release(m_pyGilState);
}

void avgDeprecationWarning(const string& sVersion, const string& sOldEntryPoint,
        const string& sNewEntryPoint)
{
    static vector<string> sWarningsIssued;
    bool bWarned = false;
    for (vector<string>::iterator it = sWarningsIssued.begin();
            it != sWarningsIssued.end(); ++it)
    {
        if (*it == sOldEntryPoint) {
            return;
        }
    }
    if (!bWarned) { 
        sWarningsIssued.push_back(sOldEntryPoint);
        
        PyFrameObject* pFrame = PyEval_GetFrame();
        int lineNo = PyCode_Addr2Line(pFrame->f_code, pFrame->f_lasti); 
                        // lineNo = PyFrame_GetLineNumber(pFrame);
        string sFName = getFilenamePart(PyString_AS_STRING(pFrame->f_code->co_filename));
        string sMsg = sFName + ":" + toString(lineNo) + ": ";
        sMsg += string(sOldEntryPoint) + " deprecated since version " + 
                string(sVersion)+"."; 
        if (sNewEntryPoint != string("")) { 
            sMsg += " Use "+string(sNewEntryPoint) + " instead."; 
        } 
        AVG_TRACE(Logger::category::DEPRECATION, Logger::severity::WARNING, sMsg); 
    } 
}

}
