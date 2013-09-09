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

#ifndef _WrapPython_H_
#define _WrapPython_H_

#include "../api.h"

#ifdef _DEBUG
#  undef _DEBUG // Don't let Python force the debug library just because we're debugging.
#  define DEBUG_UNDEFINED_FROM_WRAPPYTHON_H
#endif

// libstdc++ and python headers both define _XOPEN_SOURCE (Ubuntu 9.10, gcc 4.4, 
// python 2.6.4)
#undef _POSIX_C_SOURCE
#undef _XOPEN_SOURCE
#include <Python.h>
#include <string>

#undef HAVE_STAT
#undef HAVE_TEMPNAM
#undef PACKAGE
#undef PACKAGE_BUGREPORT
#undef PACKAGE_NAME
#undef PACKAGE_STRING
#undef PACKAGE_TARNAME
#undef PACKAGE_VERSION

#ifdef DEBUG_UNDEFINED_FROM_WRAPPYTHON_H
# undef DEBUG_UNDEFINED_FROM_WRAPPYTHON_H
# define _DEBUG
#endif

namespace avg {

class aquirePyGIL
{
public:
    aquirePyGIL();
    virtual ~aquirePyGIL();

private:
    PyGILState_STATE m_pyGilState;
};

void avgDeprecationWarning(const std::string& sVersion, const std::string& sOldEntryPoint,
        const std::string& sNewEntryPoint);

}

#endif
