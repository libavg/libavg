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

#ifndef _CmdLine_H_
#define _CmdLine_H_

#include "../api.h"
#include <string>
#include <vector>
#include <map>

namespace avg {

typedef std::map<std::string, std::string> OptionMap;

class AVG_API CmdLine {
public:
    CmdLine(int argc, char **argv);

    const OptionMap& getOptions() const;
    const std::string* getOption(const std::string& sName) const;
    int getNumArgs() const;
    const std::string* getArg(unsigned int i) const;

private:
    OptionMap m_Options;
    std::vector<std::string> m_Args;
};

}
#endif 

