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

#include "TypeRegistry.h"
#include "TypeDefinition.h"

#include "../base/MathHelper.h"
#include "../base/Exception.h"

#include <set>

using namespace std;

namespace avg {

TypeRegistry* TypeRegistry::s_pInstance = 0;

TypeRegistry::TypeRegistry()
{
}

TypeRegistry::~TypeRegistry()
{
}

TypeRegistry* TypeRegistry::get()
{
    if (!s_pInstance) {
        s_pInstance = new TypeRegistry();
    }
    return s_pInstance;
}

void TypeRegistry::registerType(const TypeDefinition& def, const char* pParentNames[])
{
    m_TypeDefs.insert(TypeDefMap::value_type(def.getName(), def));

    if (pParentNames) {
        string sChildArray[1];
        sChildArray[0] = def.getName();
        vector<string> sChildren = vectorFromCArray(1, sChildArray);
        const char **ppCurParentName = pParentNames;

        while (*ppCurParentName) {
            TypeDefinition def = getTypeDef(*ppCurParentName);
            def.addChildren(sChildren);
            updateDefinition(def);

            ++ppCurParentName;
        }
    }
}

void TypeRegistry::updateDefinition(const TypeDefinition& def)
{
    m_TypeDefs[def.getName()] = def;
}

ExportedObjectPtr TypeRegistry::createObject(const string& sType, 
        const xmlNodePtr xmlNode)
{
    const TypeDefinition& def = getTypeDef(sType);
    ArgList args(def.getDefaultArgs(), xmlNode);
    ObjectBuilder builder = def.getBuilder();
    ExportedObjectPtr pObj = builder(args);
    pObj->setTypeInfo(&def);
    return pObj;
}

ExportedObjectPtr TypeRegistry::createObject(const string& sType, const py::dict& pyDict)
{
    const TypeDefinition& def = getTypeDef(sType);
    py::dict effParams;
    effParams = pyDict;
    ArgList args(def.getDefaultArgs(), effParams);
    ObjectBuilder builder = def.getBuilder();
    ExportedObjectPtr pObj = builder(args);
    pObj->setTypeInfo(&def);
    return pObj;
}

string TypeRegistry::getDTD() const
{
    if (m_TypeDefs.empty()) {
        return string("");
    }
    
    stringstream ss;
    
    for (TypeDefMap::const_iterator defIt = m_TypeDefs.begin();
            defIt != m_TypeDefs.end(); defIt++) 
    {
        const TypeDefinition& def = defIt->second;
        if (!def.isAbstract()) {
            writeTypeDTD(def, ss);
        }
    }
   
    for (TypeDefMap::const_iterator defIt = m_TypeDefs.begin(); 
            defIt != m_TypeDefs.end(); defIt++) 
    {
        const TypeDefinition& def = defIt->second;
        if (!def.isAbstract()) {
            ss << def.getDTDElements();
        }
    }
   
    return ss.str();
}

const TypeDefinition& TypeRegistry::getTypeDef(const string& sType)
{
    TypeDefMap::const_iterator it = m_TypeDefs.find(sType);
    if (it == m_TypeDefs.end()) {
        throw (Exception (AVG_ERR_XML_NODE_UNKNOWN, 
            string("Unknown node type ") + sType + " encountered."));
    }
    return it->second;
}

void TypeRegistry::writeTypeDTD(const TypeDefinition& def, stringstream& ss) const
{
    ss << "<!ELEMENT " << def.getName() << " " << def.getDTDChildrenString() << " >\n";
    if (!def.getDefaultArgs().getArgMap().empty()) {
        ss << "<!ATTLIST " << def.getName();
        for (ArgMap::const_iterator argIt = def.getDefaultArgs().getArgMap().begin(); 
            argIt != def.getDefaultArgs().getArgMap().end(); argIt++)
        {
            string argName = argIt->first;
            string argType = (argName == "id") ? "ID" : "CDATA";
            string argRequired = def.getDefaultArgs().getArg(argName)->isRequired() ?
                    "#REQUIRED" : "#IMPLIED";
            ss << "\n    " << argName << " " << argType << " " << argRequired;
        }
        ss << " >\n";
    }
}

}
