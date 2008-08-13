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

#include "CmdLine.h"

#include <iostream>

using namespace std;
 
namespace avg {

CmdLine::CmdLine(int argc, char **argv) 
{
    for (int i = 1; i < argc; ++i) {
        string sArg(argv[i]);
        if (sArg.substr(0, 2) == "--") {
            size_t DelimPos = sArg.find('=');
            string sOptName;
            string sOptVal;
            if (DelimPos == sArg.npos) {
                sOptName = sArg.substr(2);
                sOptVal = "";
            } else {
                sOptName = sArg.substr(2, DelimPos-2);
                sOptVal = sArg.substr(DelimPos+1);
            }
            m_Options[sOptName] = sOptVal;
        } else {
            m_Args.push_back(sArg);
        }
    }
}

const OptionMap& CmdLine::getOptions() const 
{
    return m_Options;
}

const string* CmdLine::getOption(const string& sName) const
{
    OptionMap::const_iterator it = m_Options.find(sName);
    if (it == m_Options.end()) {
        return 0;
    } else {
        return &(*it).second;
    }
}

int CmdLine::getNumArgs() const
{
    return int(m_Args.size());
}
 
const string* CmdLine::getArg(unsigned int i) const
{
    if (i>=m_Args.size()) {
        return 0;
    } else {
        return &m_Args[i];
    }
}

}

