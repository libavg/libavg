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
//  Original author of this file is Nick Hebner (hebnern@gmail.com).
//

#ifndef _ArgList_H_
#define _ArgList_H_

#include "../api.h"

#include "BoostPython.h"
#include "Arg.h"

#include <libxml/parser.h>

#include <string>
#include <map>

namespace avg {

typedef std::map<std::string, ArgBasePtr> ArgMap;

class ExportedObject;

class AVG_API ArgList
{
public:
    ArgList();
    ArgList(const ArgList& argTemplates, const xmlNodePtr xmlNode);
    ArgList(const ArgList& argTemplates, const py::dict& PyDict);
    virtual ~ArgList();

    bool hasArg(const std::string& sName) const;
    const ArgBasePtr getArg(const std::string& sName) const;
   
    template<class T>
    const T& getArgVal(const std::string& sName) const;
    
    void getOverlayedArgVal(glm::vec2* pResult, const std::string& sName,
            const std::string& sOverlay1, const std::string& sOverlay2,
            const std::string& sID) const;

    const ArgMap& getArgMap() const;
    
    void setArg(const ArgBase& newArg);
    void setArgs(const ArgList& args);
    void setMembers(ExportedObject * pObj) const;
    
    void copyArgsFrom(const ArgList& argTemplates);

private:
    void setArgValue(const std::string & sName, const py::object& value);
    void setArgValue(const std::string & sName, const std::string & sValue);
    ArgMap m_Args;
};
    
template<class T>
const T& ArgList::getArgVal(const std::string& sName) const
{
    return (dynamic_cast<Arg<T>* >(&*getArg(sName)))->getValue();
}
    

}

#endif
