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
#include "Style.h"

#include "../base/MathHelper.h"
#include "../base/Exception.h"

#include <set>

using namespace std;

namespace avg {

NodeRegistry* NodeRegistry::s_pInstance = 0;

NodeRegistry::NodeRegistry()
{
}

NodeRegistry::~NodeRegistry()
{
}

NodeRegistry* NodeRegistry::get()
{
    if (!s_pInstance) {
        s_pInstance = new NodeRegistry();
    }
    return s_pInstance;
}

void NodeRegistry::registerNodeType(const NodeDefinition& def, const char* pParentNames[])
{
    m_NodeDefs.insert(NodeDefMap::value_type(def.getName(), def));

    if (pParentNames) {
        string sChildArray[1];
        sChildArray[0] = def.getName();
        vector<string> sChildren = vectorFromCArray(1, sChildArray);
        const char **ppCurParentName = pParentNames;

        while (*ppCurParentName) {
            NodeDefinition nodeDefinition = getNodeDef(*ppCurParentName);
            nodeDefinition.addChildren(sChildren);
            updateNodeDefinition(nodeDefinition);

            ++ppCurParentName;
        }
    }
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
    pNode->setTypeInfo(&def);
    return pNode;
}

NodePtr NodeRegistry::createNode(const string& sType, const py::dict& pyDict)
{
    const NodeDefinition& def = getNodeDef(sType);
    py::dict effParams;
    StylePtr pStyle;
    if (pyDict.has_key("style")) {
        py::object param = pyDict["style"];
        pyDict.attr("__delitem__")("style");
        pStyle = py::extract<StylePtr>(param);
        effParams = pStyle->mergeParams(pyDict);
    } else {
        effParams = pyDict;
    }
    ArgList args(def.getDefaultArgs(), effParams);
    NodeBuilder builder = def.getBuilder();
    NodePtr pNode = builder(args);
    pNode->setTypeInfo(&def);
    pNode->setStyle(pStyle);
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
        if (!def.isAbstract()) {
            writeNodeDTD(def, ss);
        }
    }
   
    for (NodeDefMap::const_iterator defIt = m_NodeDefs.begin(); 
            defIt != m_NodeDefs.end(); defIt++) 
    {
        const NodeDefinition& def = defIt->second;
        if (!def.isAbstract()) {
            ss << def.getDTDElements();
        }
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
