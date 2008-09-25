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

#ifndef _NodeRegistry_H_
#define _NodeRegistry_H_

#include "Node.h"
#include "ArgList.h"
#include "NodeDefinition.h"

#include <map>
#include <string>
#include <sstream>

namespace avg {

class NodeRegistry
{
public:
    NodeRegistry();
    virtual ~NodeRegistry();
    
    void registerNodeType(NodeDefinition& Def);
    const NodeDefinition& getNodeDef(const std::string& Type);
    NodePtr createNode(const std::string& Type, const xmlNodePtr xmlNode);
    NodePtr createNode(const std::string& Type, const boost::python::dict& PyDict);
    
    std::string getDTD() const;
    
private:
    void writeNodeDTD(const NodeDefinition& Def, std::stringstream& ss) const;
    
    typedef std::map<std::string, NodeDefinition> NodeDefMap;
    NodeDefMap m_NodeDefs;
};

}

#endif
