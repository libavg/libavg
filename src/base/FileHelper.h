//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2008 Ulrich von Zadow
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

#ifndef _FileHelper_H_
#define _FileHelper_H_

#include "../api.h"
#include <string>

namespace avg {
    
std::string getPath(const std::string& Filename);
std::string getFilenamePart(const std::string& Filename);
bool isAbsPath(const std::string& path);

bool fileExists(const std::string& FileName);

void readWholeFile(const std::string& sFilename, std::string& sContents);

void writeWholeFile(const std::string& sFilename, const std::string& sContent);

void copyFile(const std::string& sSourceFile, const std::string& sDestFile);

#ifdef WIN32
#define unlink _unlink
#endif

}

#endif 

