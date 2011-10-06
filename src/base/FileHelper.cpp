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

#include "FileHelper.h"
#include "ConfigMgr.h"
#include "Exception.h"

#ifndef _WIN32
#include <libgen.h>
#else
#include <direct.h>
#endif
#include <stdio.h>
#include <sys/stat.h>
#include <string.h>

#include <map>
#include <cstring>
#include <iostream>
#include <fstream>

using namespace std;

namespace avg {

string getPath(const string& sFilename)
{
    if (sFilename.length() > 0 && sFilename.at(sFilename.length()-1) == '/') {
        return sFilename;
    }
#ifdef _WIN32
    int pos = int(sFilename.find_last_of("\\/"));
    string dirName;
    if (pos >= 0) {
        dirName = sFilename.substr(0, pos+1);
    } else {
        dirName = sFilename;
    }
#else
    char * pszBuffer = strdup(sFilename.c_str());

    string dirName(dirname(pszBuffer));
    free(pszBuffer);
    dirName += "/";
#endif

    return dirName;
}

string getFilenamePart(const string& sFilename)
{
    if (sFilename.find_last_of("\\/") == 0) {
        return sFilename;
    }
#ifdef _WIN32
    int pos = int(sFilename.find_last_of("\\/"));
    string BaseName(sFilename.substr(pos+1));
#else
    char * pszBuffer = strdup(sFilename.c_str());

    string BaseName(basename(pszBuffer));
    free(pszBuffer);
#endif

    return BaseName;
}

string getExtension(const string& sFilename)
{
    int pos = int(sFilename.find_last_of("."));
    if (pos == 0) {
        return "";
    } else {
        return sFilename.substr(pos+1);
    }
}

string getCWD()
{

    char szBuf[1024];
#ifdef _WIN32
    char * pBuf = _getcwd(szBuf, 1024);
#else
    char * pBuf = getcwd(szBuf, 1024);
#endif
    return string(pBuf)+"/";
}

bool isAbsPath(const std::string& path)
{
#ifdef _WIN32
    return ((path.length() != 0) && path[1] == ':') || path[0] == '\\' || path[0] == '/';
#else
    return path[0] == '/';
#endif
    
}

bool fileExists(const string& sFilename)
{
    struct stat myStat;
    return stat(sFilename.c_str(), &myStat) != -1;
}

void readWholeFile(const string& sFilename, string& sContent)
{
    ifstream file(sFilename.c_str());
    if (!file) {
        throw Exception(AVG_ERR_FILEIO, "Opening "+sFilename+
                " for reading failed.");
    }
    vector<char> buffer(65536);
    sContent.resize(0);
    while (file) {
        file.read(&(*buffer.begin()), (streamsize)(buffer.size()));
        sContent.append(&(*buffer.begin()), (unsigned)file.gcount());
    }
    if (!file.eof() || file.bad()) {
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


void copyFile(const string& sSourceFile, const string& sDestFile)
{
    string sData;
            
    readWholeFile(sSourceFile, sData);
    writeWholeFile(sDestFile, sData);
}

}
