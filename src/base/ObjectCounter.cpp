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

#include "ObjectCounter.h"
#include "Exception.h"
#include "Logger.h"

#include <boost/thread/mutex.hpp>

#include <iostream>
#include <sstream>
#include <vector>
#include <algorithm>

#ifdef WIN32
#include <windows.h>
#include <Dbghelp.h>
#else
#include <cxxabi.h>
#endif

#define DEBUG_ALLOC 1

namespace avg {

using namespace std;

ObjectCounter* ObjectCounter::s_pObjectCounter = 0;
bool ObjectCounter::s_bDeleted = false;
boost::mutex * pCounterMutex;

void deleteObjectCounter()
{
    delete ObjectCounter::s_pObjectCounter;
    delete pCounterMutex;
    ObjectCounter::s_pObjectCounter = 0;
}

ObjectCounter::ObjectCounter()
{
}

ObjectCounter::~ObjectCounter()
{
    s_bDeleted = true;
}

ObjectCounter * ObjectCounter::get()
{
    if (!s_pObjectCounter) {
        if (s_bDeleted) {
            // This is _after_ the deleteObjectCounter has been called.
            return 0;
        } else {
            s_pObjectCounter = new ObjectCounter;
            pCounterMutex = new boost::mutex;
            atexit(deleteObjectCounter);
        }
    }
    return s_pObjectCounter;
}

void ObjectCounter::incRef(const std::type_info* pType)
{
#ifdef DEBUG_ALLOC
    lock_guard Lock(*pCounterMutex);
    TypeMap::iterator MapEntry = m_TypeMap.find(pType);
    if (MapEntry == m_TypeMap.end()) {
        m_TypeMap[pType] = 1;
    } else {
        (MapEntry->second)++;
    }
//    cerr << "incRef " << demangle(pType->name()) << ":" << m_TypeMap[pType] << endl;
#endif
}

void ObjectCounter::decRef(const std::type_info* pType)
{
#ifdef DEBUG_ALLOC
    if (!this) {
        // This happens if there are counted static objects that are deleted after 
        // s_pObjectCounter has been deleted.
        return;
    }
    lock_guard Lock(*pCounterMutex);
    TypeMap::iterator MapEntry = m_TypeMap.find(pType);
    if (MapEntry == m_TypeMap.end()) {
        cerr << "ObjectCounter for " << demangle(pType->name()) 
                << " does not exist." << endl;
        // Can't decref a type that hasn't been incref'd.
        AVG_ASSERT(false);
    } else {
        (MapEntry->second)--;
        if (MapEntry->second < 0) {
            cerr << "ObjectCounter: refcount for " << 
                    demangle(MapEntry->first->name()) <<
                    " < 0" << endl;
            AVG_ASSERT(false);
        }
    }
//    cerr << "decRef " << demangle(pType->name()) << ":" << MapEntry->second << endl;
#endif
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
    vector<string> strings;
    for (it = m_TypeMap.begin(); it != m_TypeMap.end(); ++it) {
        stringstream tempStream;
        if (it->second > 0) {
            tempStream << "  " << demangle(it->first->name()) << ": " << it->second;
            strings.push_back(tempStream.str());
        }
    }
    sort(strings.begin(), strings.end());
    for (vector<string>::iterator it=strings.begin(); it != strings.end(); ++it) {
        ss << *it << endl;
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

TypeMap ObjectCounter::getObjectCount(){
    return m_TypeMap;
}

}
