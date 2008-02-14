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

#ifndef _ArgList_H_
#define _ArgList_H_

#include "BoostPython.h"

#include "Arg.h"

#include <libxml/parser.h>

#include <string>
#include <map>

namespace avg {

typedef std::map<std::string, Arg> ArgMap;

class ArgList
{
public:
    ArgList();
    ArgList(const xmlNodePtr xmlNode);
    ArgList(const boost::python::dict& PyDict);
    virtual ~ArgList();

    const Arg& getArg(const std::string& Name) const;
    int getIntArg(const std::string& Name) const;
    double getDoubleArg(const std::string& Name) const;
    bool getBoolArg(const std::string& Name) const;
    std::string getStringArg(const std::string& Name) const;
    
    const ArgMap& getArgMap() const;
    
    void setArg(const std::string& Name, int Value, bool bRequired);
    void setArg(const std::string& Name, double Value, bool bRequired);
    void setArg(const std::string& Name, bool Value, bool bRequired);
    void setArg(const std::string& Name, const std::string& Value, bool bRequired);
    
    void setArgs(const ArgList& Args);
    
    ArgList operator+(const ArgList& OtherArgs) const;

private:
    ArgMap m_Args;
};

}

#endif
