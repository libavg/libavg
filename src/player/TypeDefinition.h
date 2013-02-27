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

#ifndef _TypeDefinition_H_
#define _TypeDefinition_H_

#include "../api.h"

#include "ArgList.h"
#include "TypeRegistry.h"

#include <map>
#include <string>

namespace avg {

class ExportedObject;
typedef boost::shared_ptr<ExportedObject> ExportedObjectPtr;
class TypeDefinition;

typedef ExportedObjectPtr (*ObjectBuilder)(const ArgList& Args);
typedef std::map<std::string, TypeDefinition> ChildMap;

class AVG_API TypeDefinition
{
public:
    TypeDefinition();
    TypeDefinition(const std::string& sName, const std::string& sBaseName="",
            ObjectBuilder pBuilder = 0);
    virtual ~TypeDefinition();
    
    const std::string& getName() const;
    ObjectBuilder getBuilder() const;
    const ArgList& getDefaultArgs() const;
    const std::string& getDTDElements() const;
    std::string getDTDChildrenString() const;
    bool isChildAllowed(const std::string& sChild) const;
    bool hasChildren() const;
    bool isAbstract() const;
    
    TypeDefinition& addArg(const ArgBase& newArg);
    TypeDefinition& addDTDElements(const std::string& s);
    TypeDefinition& addChildren(const std::vector<std::string>& sChildren);

private:
    std::string m_sName;
    ObjectBuilder m_pBuilder;
    ArgList m_Args;
    std::string m_sDTDElements;
    std::vector<std::string> m_sChildren;

};

}

#endif
