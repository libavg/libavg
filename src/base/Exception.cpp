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

#include "Exception.h"
#include "Backtrace.h"
#include "Logger.h"
#include "OSHelper.h"

#include <cstdlib>
#include <sstream>

using namespace std;

namespace avg {

Exception::Exception(int code, const string& sErr)
    : m_Code (code),
      m_sErr (sErr)
{
}

Exception::Exception(const Exception& ex)
    : m_Code (ex.getCode()),
      m_sErr (ex.getStr())
{
}

Exception::~Exception()
{
}

int Exception::getCode() const
{
    return m_Code;
}

const string& Exception::getStr() const
{
    return m_sErr;
}

void fatalError(const string& sMsg)
{
    AVG_TRACE(Logger::ERROR, "Internal error: "+sMsg+" Aborting.");
    exit(-1);
}

void debugBreak()
{
#ifdef _WIN32
    __asm int 3;
#else
    asm("int $3");
#endif
}

void avgAssert(bool b, const char * pszFile, int line)
{
    if (!b) {
        string sDummy;
        static bool bBreak = getEnv("AVG_BREAK_ON_ASSERT", sDummy);
        if (bBreak) {
            debugBreak();
        } else {
            stringstream ss;
            ss << "Assertion failed in " << pszFile << ": " << line;
            dumpBacktrace();
            throw(Exception(AVG_ERR_ASSERT_FAILED, ss.str()));
        }
    }
}

}
