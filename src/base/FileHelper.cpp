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

#include "FileHelper.h"
#include "ConfigMgr.h"
#include "Exception.h"

#ifndef _WIN32
#include <libgen.h>
#endif
#include <stdio.h>
#include <sys/stat.h>

#include <map>
#include <iostream>
#include <fstream>

using namespace std;

namespace avg {

string getPath(const string& Filename)
{
    if (Filename.length() > 0 && Filename.at(Filename.length()-1) == '/') {
        return Filename;
    }
#ifdef _WIN32
    int pos = int(Filename.find_last_of("\\"));
    string DirName;
    if (pos >= 0) {
        DirName = Filename.substr(0, pos);
    } else {
        DirName = Filename;
    }
#else
    char * pszBuffer = strdup(Filename.c_str());

    string DirName(dirname(pszBuffer));
    free(pszBuffer);
#endif

    DirName += "/";
    return DirName;
}

bool fileExists(const std::string& FileName) {
    struct stat myStat;
    return stat(FileName.c_str(), &myStat) != -1;
}

void readWholeFile(const string& sFilename, string& sContent)
{
    ifstream File(sFilename.c_str());
    if (!File) {
        throw Exception(AVG_ERR_FILEIO, "Opening "+sFilename+
                " for reading failed.");
    }
    vector<char> Buffer(65536);
    sContent.resize(0);
    while (File) {
        File.read(&(*Buffer.begin()), (streamsize)(Buffer.size()));
        sContent.append(&(*Buffer.begin()),File.gcount());
    }
    if (!File.eof() || File.bad()) {
        throw Exception(AVG_ERR_FILEIO, "Reading "+sFilename+
                " failed.");
    }
}

void writeWholeFile(const string& sFilename, const string& sContent)
{
    ofstream outFile(sFilename.c_str());
    if (!outFile) {
        throw Exception(AVG_ERR_FILEIO, "Opening "+sFilename+
                " for writing failed.");
    }
    outFile << sContent;
}

}
