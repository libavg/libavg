//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2006 Ulrich von Zadow
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

#include "OSHelper.h"
#include "FileHelper.h"
#include "Logger.h"
#include "FileHelper.h"

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef ERROR
#undef WARNING
#elif defined(__APPLE__)
#include <mach-o/dyld.h>
#endif

using namespace std;

namespace avg {

#ifdef _WIN32
string getWinErrMsg(unsigned err) 
{
    LPVOID lpMsgBuf;
    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
                FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &lpMsgBuf,
        0, NULL );
    string sMsg((char*)lpMsgBuf);
    LocalFree(lpMsgBuf);
    return sMsg;
}
#endif

string getAvgLibPath()
{
#if defined(_WIN32)
    HMODULE hModule = GetModuleHandle("avg.pyd");
/*
    if (hModule == 0) {
        AVG_TRACE(Logger::ERROR, "getAvgLibPath(): " << getWinErrMsg(GetLastError()));
        exit(5);
    }
*/
    char szFilename[1024];
    DWORD ok = GetModuleFileName(hModule, szFilename, sizeof(szFilename));
    if (ok == 0) {
        AVG_TRACE(Logger::ERROR, "getAvgLibPath(): " << getWinErrMsg(GetLastError()));
        exit(5);
    } 
    string sPath=getPath(szFilename);
    return sPath;
#elif defined(__APPLE__)
    // We need to iterate through all images attached to the current executable
    // and figure out which one is the one we are interested in.
    uint32_t numImages = _dyld_image_count();
    for (uint32_t i=0; i<numImages; i++) {
         const char * pszImageName = _dyld_get_image_name(i);
         cerr << pszImageName << endl;
         string sFilePart=getFilenamePart(pszImageName);
         if (sFilePart == "avg.so" || sFilePart == "avg.0.0.0.so") {
             return getPath(pszImageName);
         }
    }
    char Path[1024];
    uint32_t PathLen = sizeof(Path);
    _NSGetExecutablePath(Path, &PathLen);
    return getPath(Path);
#else
    // For a linux solution, see http://www.autopackage.org/docs/binreloc/
    return "";
#endif
}

bool getEnv(const string & sName, string & sVal)
{
    const char * pszVal = getenv(sName.c_str());
    if (pszVal) {
        sVal = pszVal;
    }
    return (pszVal != 0);
}

void setEnv(const string & sName, const string & sVal)
{
#ifdef _WIN32
    SetEnvironmentVariable(sName.c_str(), sVal.c_str());
#else
    setenv(sName.c_str(), sVal.c_str(), true);
#endif
}

}
