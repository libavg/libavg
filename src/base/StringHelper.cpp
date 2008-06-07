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

#include "StringHelper.h"

#include <stdlib.h>

using namespace std;

namespace avg {

int stringToInt(const std::string& s)
{
    char * errStr;
    int ret = strtol(s.c_str(), &errStr, 10);
    if (s != "" && *errStr == 0) {
        return ret;
    } else {
        throw (Exception(AVG_ERR_TYPE, string("Could not convert '")+s+"' to int."));
    }
}

double stringToDouble(const std::string& s)
{
    char * errStr;
    double ret = strtod(s.c_str(), &errStr);
    if (s != "" && *errStr == 0) {
        return ret;
    } else {
        throw (Exception(AVG_ERR_TYPE, string("Could not convert '")+s
                +"' to floating point."));
    }
    
}

bool stringToBool(const std::string& s)
{
    // avg usually wants xml attributes in lowercase, but python only
    // sees 'True' as true, so we'll accept that too. Also, python 2.3
    // has 1 as true, so that has to be ok too.
    if (s == "True" || s == "true" || s == "1") {
        return true;
    }
    if (s == "False" || s == "false" || s == "0") {
        return false;
    }
    throw (Exception(AVG_ERR_TYPE, string("Could not convert ")+s+" to bool."));
}

std::string removeStartEndSpaces(const std::string& s)
{
    string sResult = s;
    while (sResult[0] == ' ' || sResult[0] == '\n' || sResult[0] == '\r' 
            || sResult[0] == '\t') 
    {
        sResult.erase(0, 1);
    }
    char c = sResult[sResult.length()-1];
    while (c == ' ' || c == '\n' || c == '\r' || c == '\t') {
        sResult.erase(sResult.length()-1, 1);
        c = sResult[sResult.length()-1];
    }
    return sResult;
}

}

