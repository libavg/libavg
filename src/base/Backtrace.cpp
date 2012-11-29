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

#include "Backtrace.h"

#include "StringHelper.h"

#ifndef _WIN32
#include <execinfo.h>
#include <cxxabi.h>
#endif

#include <stdlib.h>
#include <iostream>
#include <stdio.h>

using namespace std;

namespace avg {

void dumpBacktrace()
{
#ifndef _WIN32
    vector<string> sFuncs;
    getBacktrace(sFuncs);
    vector<string>::iterator it = sFuncs.begin();
    ++it;
    for (; it != sFuncs.end(); ++it) {
        cerr << "  " << *it << endl;
    }
#endif
}

string funcNameFromLine(const string& sLine)
{
    try {
#ifdef __APPLE__
        string::size_type addressPos = sLine.find("0x");
        string::size_type namePos = sLine.find(" ", addressPos);
        namePos++;
        string::size_type nameEndPos = sLine.find(" ", namePos);
#else
        string::size_type namePos = sLine.find("(");
        namePos++;
        string::size_type nameEndPos = sLine.find_first_of(")+", namePos);
#endif
        return sLine.substr(namePos, nameEndPos-namePos);
    } catch (exception&) {
        return sLine;
    }
}

void consolidateRepeatedLines(vector<string>& sFuncs, unsigned& i, unsigned numSameLines)
{
    unsigned firstSameLine = i - numSameLines;
    sFuncs[firstSameLine+1] = "    [...]";
    sFuncs.erase(sFuncs.begin()+firstSameLine+2, sFuncs.begin()+i-1);
    i = firstSameLine + 3;
}

void getBacktrace(vector<string>& sFuncs)
{
#ifndef _WIN32
    void* callstack[128];
    int numFrames = backtrace(callstack, 128);
    char** ppszLines = backtrace_symbols(callstack, numFrames);
    for (int i = 1; i < numFrames; ++i) {
        string sLine = ppszLines[i];
        string sFuncName = funcNameFromLine(sLine);
        int result;
        char * pszDemangledFuncName = abi::__cxa_demangle(sFuncName.c_str(), 0, 0,
                &result);
        if (!result) {
            sFuncName = pszDemangledFuncName;
            free(pszDemangledFuncName);
        }
        char szLineNum[10];
        sprintf(szLineNum, "%3d", i);
        sFuncs.push_back(string(szLineNum)+" "+sFuncName);
    }
    free(ppszLines);

    unsigned numSameLines = 1;
    unsigned i = 1;
    for (i = 1; i < sFuncs.size(); ++i) {
        if (sFuncs[i].substr(4, string::npos) == sFuncs[i-1].substr(4, string::npos)) {
            numSameLines++;
        } else {
            if (numSameLines > 3) {
                consolidateRepeatedLines(sFuncs, i, numSameLines);
            }
            numSameLines = 1;
        }
    }
    if (numSameLines > 2) {
        consolidateRepeatedLines(sFuncs, i, numSameLines);
    }
#endif
}

}

