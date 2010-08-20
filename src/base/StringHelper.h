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

#ifndef _StringHelper_H_
#define _StringHelper_H_

#include "../api.h"
#include "Exception.h"
#include "Point.h"
#include "Triple.h"

#include <string>
#include <sstream>
#include <typeinfo>
#include <iostream>

#ifdef __GNUC__
#include <cxxabi.h>
#endif

namespace avg {

bool isWhitespace(const std::string& s);
void skipWhitespace(std::istream& is);
void skipToken(std::istream& is, char token);

template<class NUM>
std::istream& operator>>(std::istream& is, Point<NUM>& p)
{
    skipToken(is, '(');
    is >> p.x;
    skipToken(is, ',');
    is >> p.y;
    skipToken(is, ')');
    return is;
}

template<class T>
std::istream& operator >>(std::istream& is, std::vector<T>& v)
{
    skipToken(is, '(');
    skipWhitespace(is);
    int c = is.peek();
    if (c == ')') {
        is.ignore();
        return is;
    }
    bool bDone = false;
    do {
        T elem;
        is >> elem;
        v.push_back(elem);
        skipWhitespace(is);
        int c = is.peek();
        switch(c) {
            case ',':
                is.ignore();
                break;
            case ')':
                bDone = true;
                is.ignore();
                break;
            default:
                is.setstate(std::ios::failbit);
                bDone = true;
        }
    } while (!bDone);
    return is;
}

int stringToInt(const std::string& s);
double stringToDouble(const std::string& s);
bool stringToBool(const std::string& s);
DPoint stringToDPoint(const std::string& s);
IntTriple stringToIntTriple(const std::string& s);

std::string removeStartEndSpaces(const std::string& s);

std::string tolower(const std::string& s);

bool equalIgnoreCase(const std::string& s1, const std::string& s2);

template<class T>
std::string toString(const T& i)
{
    std::stringstream stream;
    stream << i;
    return stream.str();
}

template<class T>
std::string getFriendlyTypeName(const T& dummy)
{
    std::string sTypeName = typeid(T).name();
#ifdef __GNUC__
    int status;
    char* const pClearName = abi::__cxa_demangle (sTypeName.c_str(), 0, 0, &status);
    if (status == 0) {
        sTypeName = pClearName;
    }
#endif
    return sTypeName;
}

template<class T>
void fromString(const std::string& s, T& result)
{
    std::stringstream stream(s);
    bool bOk = (stream >> result) != 0;
    if (bOk) {
        std::string sLeftover;
        stream >> sLeftover;
        bOk = isWhitespace(sLeftover);
    }
    if (!bOk) {
        std::string sTypeName = getFriendlyTypeName(result);
        throw (Exception(AVG_ERR_TYPE, std::string("Could not convert '")+s
                + "' to "+sTypeName+"."));
    }
}


}



#endif
