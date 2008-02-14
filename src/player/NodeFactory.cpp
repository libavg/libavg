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

#include "NodeFactory.h"
#include "NodeDefinition.h"

#include "../base/Exception.h"

#include <set>

using namespace std;

namespace avg {

NodeFactory::NodeFactory()
{
}

NodeFactory::~NodeFactory()
{
}

void NodeFactory::registerNodeType(NodeDefinition& Def)
{
    m_NodeDefs.insert(NodeDefMap::value_type(Def.getName(), Def));
}

NodePtr NodeFactory::createNode(const string& Type, const ArgList& Args, Player* pPlayer)
{
    NodeDefMap::const_iterator it = m_NodeDefs.find(Type);
    if (it == m_NodeDefs.end()) {
        throw (Exception (AVG_ERR_XML_NODE_UNKNOWN, 
            string("Unknown node type ") + Type + " encountered."));
    }
    const NodeDefinition& Def = it->second;
    if (!Def.validateArgs(Args)) {
        throw (Exception (AVG_ERR_INVALID_ARGS, "Invalid arguments encountered"));
    }
    ArgList fullArgs = Args + Def.getDefaultArgs();
    NodeBuilder builder = Def.getBuilder();
    if (!builder) {
        throw (Exception (AVG_ERR_NO_BUILDER, string("No node builder defined for ")+Type));
    }
    return builder(fullArgs, pPlayer);
}

string NodeFactory::getDTD() const
{
    if (m_NodeDefs.empty()) {
        return string("");
    }
    
    stringstream ss;
    
    NodeDefMap::const_iterator it = m_NodeDefs.begin();
    ss << "<!ENTITY % anyNode \"" << it->first;
    for(it++; it != m_NodeDefs.end(); it++) {
        ss << "|" << it->first;
    }
    ss << "\" >\n";
    
    set<string> nodesWritten;
    
    for(NodeDefMap::const_iterator defIt = m_NodeDefs.begin(); defIt != m_NodeDefs.end(); defIt++) {
        const NodeDefinition& Def = defIt->second;
        
        writeNodeDTD(Def, ss);
        nodesWritten.insert(Def.getName());
        
        for(ChildMap::const_iterator childIt = Def.getChildren().begin(); 
            childIt != Def.getChildren().end(); childIt++) {
            const NodeDefinition& childDef = childIt->second;
            if (childDef.getName()[0] != '#' && childDef.getName()[0] != '%') {
                set<string>::iterator nit = nodesWritten.find(childDef.getName());
                if (nit == nodesWritten.end()) {
                    writeNodeDTD(childDef, ss);
                    nodesWritten.insert(childDef.getName());
                }
            }
        }
    }
    
    return ss.str();
}

void NodeFactory::writeNodeDTD(const NodeDefinition& Def, stringstream& ss) const
{
    stringstream cs;
    if (!Def.getChildren().empty()) {
        ChildMap::const_iterator childIt = Def.getChildren().begin();
        cs << "(" << childIt->first;
        for(childIt++; childIt != Def.getChildren().end(); childIt++) {
            const NodeDefinition& Child = childIt->second;
            cs << "|" << Child.getName();
        }
        cs << ")*";
    } else {
        cs << "EMPTY";
    }
    
    ss << "<!ELEMENT " << Def.getName() << " " << cs.str() << " >\n";
    if (!Def.getDefaultArgs().getArgMap().empty()) {
        ss << "<!ATTLIST " << Def.getName();
        for(ArgMap::const_iterator argIt = Def.getDefaultArgs().getArgMap().begin(); 
            argIt !=  Def.getDefaultArgs().getArgMap().end(); argIt++)
        {
            string argName = argIt->first;
            string argType = (argName == "id") ? "ID" : "CDATA";
            string argRequired = Def.getDefaultArgs().getArg(argName).isRequired() ?
                    "#REQUIRED" : "#IMPLIED";
            ss << "\n    " << argName << " " << argType << " " << argRequired;
        }
        ss << " >\n";
    }
}

}
