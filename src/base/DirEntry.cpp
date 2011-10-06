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

#include "DirEntry.h"

using namespace std;

namespace avg {

#ifdef _WIN32
DirEntry::DirEntry(string sDirName, const _finddata_t& findData)
    : m_sDirName(sDirName),
      m_FindData(findData)
{
}

#else
DirEntry::DirEntry(string sDirName, dirent * pEntry)
    : m_sDirName(sDirName),
      m_pEntry(pEntry)  
{
}
#endif

DirEntry::~DirEntry()
{
}

std::string DirEntry::getName()
{
#ifdef _WIN32
    return m_FindData.name;
#else
    return m_pEntry->d_name;
#endif
}

void DirEntry::remove()
{
#ifdef _WIN32
    ::_unlink((m_sDirName+"\\"+m_FindData.name).c_str());
#else
    ::unlink((m_sDirName+"/"+m_pEntry->d_name).c_str());
#endif
}

}
