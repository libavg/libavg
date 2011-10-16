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

#include "StringHelper.h"

#include <stdlib.h>
#include <ctype.h>
#include <algorithm>
#include <cstdio>
#include <iterator>

using namespace std;

namespace avg {

bool isWhitespace(const string& s)
{
    return s.find_first_not_of(" \n\r\t") == s.npos;
}

void skipWhitespace(std::istream& is)
{
    string sWhitespace(" \n\r\t");
    bool bWhitespace;
    do {
        int i = is.peek();
        if (i == EOF) {
            bWhitespace = false;
        } else {
            bWhitespace = (sWhitespace.find(char(i)) != sWhitespace.npos);
        }
        if (bWhitespace) {
            is.ignore();
        }
    } while (bWhitespace);
}

void skipToken(std::istream& is, char token)
{
    skipWhitespace(is);
    int i = is.peek();
    if (i == token) {
        is.ignore();
    } else {
        is.setstate(ios::failbit);
    }
}

int stringToInt(const string& s)
{
    int i;
    fromString(s, i);
    return i;
}

double stringToDouble(const string& s)
{
    double d;
    fromString(s, d);
    return d;
}

bool stringToBool(const string& s)
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

DPoint stringToDPoint(const std::string& s)
{
    DPoint pt;
    fromString(s, pt);
    return pt;
}

IntTriple stringToIntTriple(const std::string& s)
{
    IntTriple pt;
    fromString(s, pt);
    return pt;
}

std::string removeStartEndSpaces(const string& s)
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

string toLowerCase(const string& s)
{
    string sResult;
    for (unsigned i=0; i<s.length(); ++i) {
        sResult.push_back(::tolower(s[i]));
    }
    return sResult;
}

bool equalIgnoreCase(const string& s1, const string& s2)
{
    if (s1.length() != s2.length()) {
        return false;
    }
    string sUpper1;
    string sUpper2;
    transform(s1.begin(), s1.end(), std::back_inserter(sUpper1), (int(*)(int)) toupper);
    transform(s2.begin(), s2.end(), std::back_inserter(sUpper2), (int(*)(int)) toupper);
    return sUpper1 == sUpper2;
}

}

