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

#ifndef _TypeRegistry_H_
#define _TypeRegistry_H_

#include "../api.h"
#include "WrapPython.h"
#include "Node.h"
#include "ArgList.h"
#include "TypeDefinition.h"

#include <map>
#include <string>
#include <sstream>

namespace avg {

class AVG_API TypeRegistry
{
public:
    virtual ~TypeRegistry();
    static TypeRegistry* get();
    
    void registerType(const TypeDefinition& def, const char* pParentNames[] = 0);
    void updateDefinition(const TypeDefinition& def);
    const TypeDefinition& getTypeDef(const std::string& Type);
    ExportedObjectPtr createObject(const std::string& Type, const xmlNodePtr xmlNode);
    ExportedObjectPtr createObject(const std::string& Type, const py::dict& PyDict);
    
    std::string getDTD() const;
    
private:
    TypeRegistry();
    void writeTypeDTD(const TypeDefinition& def, std::stringstream& ss) const;
    
    typedef std::map<std::string, TypeDefinition> TypeDefMap;
    TypeDefMap m_TypeDefs;

    static TypeRegistry* s_pInstance;
};

}

#endif
