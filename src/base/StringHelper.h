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

#include <string>
#include <sstream>
#include <typeinfo>
#include <iostream>

#ifdef __GNUC__
#include <cxxabi.h>
#endif

namespace avg {

bool isWhitespace(const std::string& s);

int stringToInt(const std::string& s);
double stringToDouble(const std::string& s);
bool stringToBool(const std::string& s);

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
        std::string sTypeName = typeid(T).name();
#ifdef __GNUC__
        int status;
        char* const pClearName = abi::__cxa_demangle (sTypeName.c_str(), 0, 0, &status);
        if (status == 0) {
            sTypeName = pClearName;
        }
#endif
        throw (Exception(AVG_ERR_TYPE, std::string("Could not convert '")+s
                + "' to "+sTypeName+"."));
    }
}

}



#endif
