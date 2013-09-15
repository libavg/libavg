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

#include "UTF8String.h"

#include <iostream>

using namespace std;

namespace avg {

UTF8String::UTF8String()
{
}

UTF8String::UTF8String(const string& s)
    : string(s)
{
}

UTF8String::UTF8String(const char * psz)
    : string(psz)
{
    
}

UTF8String& UTF8String::operator =(const string& s)
{
    *dynamic_cast<string*>(this) = s;
    return *this;
}

UTF8String& UTF8String::operator =(const char* psz)
{
    *dynamic_cast<string*>(this) = psz;
    return *this;
}

std::size_t hash_value(const avg::UTF8String& x)
{
    boost::hash<std::string> hasher;
    return hasher(x);
};
}
