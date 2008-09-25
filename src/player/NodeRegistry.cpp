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
//  Original author of this file is Nick Hebner (hebnern@gmail.com).
//

#include "NodeRegistry.h"
#include "NodeDefinition.h"

#include "../base/Exception.h"

#include <set>

using namespace std;

namespace avg {

NodeRegistry::NodeRegistry()
{
}

NodeRegistry::~NodeRegistry()
{
}

void NodeRegistry::registerNodeType(NodeDefinition& Def)
{
    m_NodeDefs.insert(NodeDefMap::value_type(Def.getName(), Def));
}

NodePtr NodeRegistry::createNode(const string& Type, const xmlNodePtr xmlNode)
{
    const NodeDefinition& Def = getNodeDef(Type);
    ArgList Args(Def.getDefaultArgs(), xmlNode);
    NodeBuilder builder = Def.getBuilder();
    NodePtr pNode = builder(Args, true);
    pNode->setThis(pNode, &Def);
    return pNode;
}

NodePtr NodeRegistry::createNode(const string& Type, const boost::python::dict& PyDict)
{
    const NodeDefinition& Def = getNodeDef(Type);
    ArgList Args(Def.getDefaultArgs(), PyDict);
    NodeBuilder builder = Def.getBuilder();
    NodePtr pNode = builder(Args, true);
    pNode->setThis(pNode, &Def);
    return pNode;
}

string NodeRegistry::getDTD() const
{
    if (m_NodeDefs.empty()) {
        return string("");
    }
    
    stringstream ss;
    
    for(NodeDefMap::const_iterator defIt = m_NodeDefs.begin();
            defIt != m_NodeDefs.end(); defIt++) 
    {
        const NodeDefinition& Def = defIt->second;
        writeNodeDTD(Def, ss);
    }
   
    for(NodeDefMap::const_iterator defIt = m_NodeDefs.begin(); 
            defIt != m_NodeDefs.end(); defIt++) 
    {
        const NodeDefinition& Def = defIt->second;
        ss << Def.getDTDElements();
    }
   
    return ss.str();
}

const NodeDefinition& NodeRegistry::getNodeDef(const string& Type)
{
    NodeDefMap::const_iterator it = m_NodeDefs.find(Type);
    if (it == m_NodeDefs.end()) {
        throw (Exception (AVG_ERR_XML_NODE_UNKNOWN, 
            string("Unknown node type ") + Type + " encountered."));
    }
    return it->second;
}

void NodeRegistry::writeNodeDTD(const NodeDefinition& Def, stringstream& ss) const
{
    ss << "<!ELEMENT " << Def.getName() << " " << Def.getChildren() << " >\n";
    if (!Def.getDefaultArgs().getArgMap().empty()) {
        ss << "<!ATTLIST " << Def.getName();
        for(ArgMap::const_iterator argIt = Def.getDefaultArgs().getArgMap().begin(); 
            argIt !=  Def.getDefaultArgs().getArgMap().end(); argIt++)
        {
            string argName = argIt->first;
            string argType = (argName == "id") ? "ID" : "CDATA";
            string argRequired = Def.getDefaultArgs().getArg(argName)->isRequired() ?
                    "#REQUIRED" : "#IMPLIED";
            ss << "\n    " << argName << " " << argType << " " << argRequired;
        }
        ss << " >\n";
    }
}

}
