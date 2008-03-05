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

#ifndef _NodeDefinition_H_
#define _NodeDefinition_H_

#include "Node.h"

#include <map>
#include <string>

namespace avg {

class ArgList;
class Player;

typedef NodePtr (*NodeBuilder)(const ArgList& Args, Player* pPlayer);
typedef std::map<std::string, NodeDefinition> ChildMap;

class NodeDefinition
{
public:
    NodeDefinition(const std::string& Name, NodeBuilder pBuilder = 0);
    virtual ~NodeDefinition();
    
    const std::string& getName() const;
    NodeBuilder getBuilder() const;
    const ArgList& getDefaultArgs() const;
    const ChildMap& getChildren() const;
    
    NodeDefinition& extendDefinition(const NodeDefinition& Def);
    NodeDefinition& addArg(const ArgBase& newArg);
    NodeDefinition& addChild(const NodeDefinition& Def);
    NodeDefinition& addChildren(const ChildMap& Children);

private:
    std::string m_sName;
    NodeBuilder m_pBuilder;
    ArgList m_Args;
    ChildMap m_Children;
};

}

#endif
