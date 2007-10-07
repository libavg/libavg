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

#include "ObjectCounter.h"

#include <boost/thread/mutex.hpp>

#include <assert.h>
#include <iostream>
#include <sstream>

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <Dbghelp.h>
#else
#include <cxxabi.h>
#endif

#define DEBUG_ALLOC 1

namespace avg {

using namespace std;

ObjectCounter* ObjectCounter::m_pObjectCounter = 0;
boost::mutex CounterMutex;

ObjectCounter::ObjectCounter()
{
}

ObjectCounter::~ObjectCounter()
{
}

ObjectCounter * ObjectCounter::get()
{
#ifdef DEBUG_ALLOC
    if (!m_pObjectCounter) {
        boost::mutex::scoped_lock Lock(CounterMutex);
        m_pObjectCounter = new ObjectCounter;
    }
    return m_pObjectCounter;
#endif
}

void ObjectCounter::incRef(const std::type_info* pType)
{
#ifdef DEBUG_ALLOC
    boost::mutex::scoped_lock Lock(CounterMutex);
    TypeMap::iterator MapEntry = m_TypeMap.find(pType);
    if (MapEntry == m_TypeMap.end()) {
        m_TypeMap[pType] = 1;
    } else {
        (MapEntry->second)++;
    }
#endif
}

void ObjectCounter::decRef(const std::type_info* pType)
{
    boost::mutex::scoped_lock Lock(CounterMutex);
    TypeMap::iterator MapEntry = m_TypeMap.find(pType);
    if (MapEntry == m_TypeMap.end()) {
        // Can't decref a type that hasn't been incref'd.
        assert (false);
    } else {
        (MapEntry->second)--;
        if (MapEntry->second < 0) {
            cerr << "ObjectCounter: refcount for " << 
                    demangle(MapEntry->first->name()) <<
                    " < 0" << endl;
            assert (false);
        }
    }
}
    
int ObjectCounter::getCount(const std::type_info* pType)
{
    TypeMap::iterator MapEntry = m_TypeMap.find(pType);
    if (MapEntry == m_TypeMap.end()) {
        return 0;
    } else {
        return MapEntry->second;
    }
    
}

std::string ObjectCounter::dump()
{
    stringstream ss;
    ss << "Object dump: " << endl;
    TypeMap::iterator it;
    for (it = m_TypeMap.begin(); it != m_TypeMap.end(); ++it) {
        if (it->second > 0) {
            ss << "  " << demangle(it->first->name()) << ": " << it->second << endl;
        }
    }
    return ss.str();
}

string ObjectCounter::demangle(string s)
{
    int rc;
    string sResult;
#ifdef _WIN32
    char szDemangledName[2048];
    rc = int(UnDecorateSymbolName(s.c_str(), szDemangledName, sizeof(szDemangledName), 
            UNDNAME_COMPLETE));
    if (rc) {
        sResult = szDemangledName;
    } else {
        int error = GetLastError();
        printf("UnDecorateSymbolName returned error %d\n", error);
        sResult = s;
    }
#else
    char * pszDemangled = abi::__cxa_demangle(s.c_str(), 0, 0, &rc);
    if (rc) {
        sResult = s;
    } else {
        sResult = pszDemangled;
    }
    if (pszDemangled) {
        free(pszDemangled);
    }
#endif
    return sResult;
}

}
