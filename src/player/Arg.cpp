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
//  Original author of this file is Nick Hebner (hebnern@gmail.com).
//

#include "Arg.h"

#include "../base/Exception.h"

#include <stdlib.h>

using namespace std;

namespace avg {

Arg::Arg(string Name, string Value, bool bRequired)
    : m_Name(Name),
      m_Value(Value),
      m_bRequired(bRequired)
{
}

Arg::~Arg()
{
}
    
string Arg::getName() const
{
    return m_Name;
}

bool Arg::isRequired() const
{
    return m_bRequired;
}

int Arg::toInt() const
{
    char * errStr;
    const char * valStr = m_Value.c_str();
    int ret = strtol(valStr, &errStr, 10);
    if (ret == 0 && errStr == valStr) {
        throw Exception(AVG_ERR_NO_ARG, 
                string("Error in conversion of '")+m_Value+"' to int");
    }
    return ret;
}

double Arg::toDouble() const
{
    char * errStr;
    const char * valStr = m_Value.c_str();
    double ret = strtod(valStr, &errStr);
    if (ret == 0 && errStr == valStr) {
        throw Exception(AVG_ERR_NO_ARG, 
                string("Error in conversion of '")+m_Value+"' to double");
    }
    return ret;
}

bool Arg::toBool() const
{
    return (m_Value == "True" || m_Value == "true" || m_Value == "1");
}

string Arg::toString() const
{
    return m_Value;
}

}
