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

void NodeRegistry::registerNodeType(const NodeDefinition& def)
{
    m_NodeDefs.insert(NodeDefMap::value_type(def.getName(), def));
}

void NodeRegistry::updateNodeDefinition(const NodeDefinition& def)
{
    m_NodeDefs[def.getName()] = def;
}

NodePtr NodeRegistry::createNode(const string& sType, const xmlNodePtr xmlNode)
{
    const NodeDefinition& def = getNodeDef(sType);
    ArgList args(def.getDefaultArgs(), xmlNode);
    NodeBuilder builder = def.getBuilder();
    NodePtr pNode = builder(args);
    pNode->setThis(pNode, &def);
    return pNode;
}

NodePtr NodeRegistry::createNode(const string& sType, const boost::python::dict& pyDict)
{
    const NodeDefinition& def = getNodeDef(sType);
    ArgList args(def.getDefaultArgs(), pyDict);
    NodeBuilder builder = def.getBuilder();
    NodePtr pNode = builder(args);
    pNode->setThis(pNode, &def);
    return pNode;
}

string NodeRegistry::getDTD() const
{
    if (m_NodeDefs.empty()) {
        return string("");
    }
    
    stringstream ss;
    
    for (NodeDefMap::const_iterator defIt = m_NodeDefs.begin();
            defIt != m_NodeDefs.end(); defIt++) 
    {
        const NodeDefinition& def = defIt->second;
        writeNodeDTD(def, ss);
    }
   
    for (NodeDefMap::const_iterator defIt = m_NodeDefs.begin(); 
            defIt != m_NodeDefs.end(); defIt++) 
    {
        const NodeDefinition& def = defIt->second;
        ss << def.getDTDElements();
    }
   
    return ss.str();
}

const NodeDefinition& NodeRegistry::getNodeDef(const string& sType)
{
    NodeDefMap::const_iterator it = m_NodeDefs.find(sType);
    if (it == m_NodeDefs.end()) {
        throw (Exception (AVG_ERR_XML_NODE_UNKNOWN, 
            string("Unknown node type ") + sType + " encountered."));
    }
    return it->second;
}

void NodeRegistry::writeNodeDTD(const NodeDefinition& def, stringstream& ss) const
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
