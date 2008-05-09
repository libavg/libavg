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

#include "Directory.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif
#include <sys/stat.h>
#include <iostream>

using namespace std;

namespace avg {

Directory::Directory(std::string sName)
    : m_sName(sName)
{
#ifdef _WIN32
    m_hFile = -1;
#else
    m_pDir = 0;
#endif
}

Directory::~Directory()
{
#ifdef _WIN32
    _findclose(m_hFile);
#else
    if (m_pDir) {
        closedir(m_pDir);
    }
#endif
}

int Directory::open(bool bCreateIfMissing)
{
#ifdef _WIN32
    m_hFile = _findfirst((m_sName+"/*").c_str(), &m_FindData);
    if(m_hFile == -1L) {
        if (bCreateIfMissing) {
            int err = CreateDirectory(m_sName.c_str(), 0);
            if (err == 0) {
                return -1;
            } else {
                m_hFile = _findfirst(m_sName.c_str(), &m_FindData);
                m_bFirstFile = true;
                return 0;
            }
        } else {
            return -1;
        }
    } else {
        m_bFirstFile = true;
        return 0;
    }
#else
    m_pDir = opendir(m_sName.c_str());
    if (!m_pDir) {
        if (bCreateIfMissing) {
            int err = mkdir(m_sName.c_str(), 
                    S_IRWXU | S_IRWXG | S_IRWXO);
            if (err) {
                return err;
            } else {
                m_pDir = opendir(m_sName.c_str());
                return 0;
            }
        } else {
            return -1;
        }
    } else {
        return 0;
    }
#endif    
}

DirEntryPtr Directory::getNextEntry()
{
#ifdef _WIN32
    if (!m_bFirstFile) {
        int rc = _findnext(m_hFile, &m_FindData);
        if (rc == -1) {
            return DirEntryPtr();
        } 
    }
    m_bFirstFile = false;
    return DirEntryPtr(new DirEntry(m_sName, m_FindData));
#else
    dirent * pDirent;
    pDirent = readdir(m_pDir);
    if (pDirent) {
        return DirEntryPtr(new DirEntry(m_sName, pDirent));
    } else {
        return DirEntryPtr();
    }
#endif
}
    
void Directory::empty()
{
    DirEntryPtr pEntry;
    do {
        pEntry = getNextEntry();
        if (pEntry) {
            pEntry->remove();
        }
    } while (pEntry);
}

}
