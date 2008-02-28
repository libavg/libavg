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

#include "NodeDefinition.h"

#include "../base/Logger.h"

using namespace std;

namespace avg {

NodeDefinition::NodeDefinition(const string& Name, NodeBuilder pBuilder)
    : m_sName(Name),
      m_pBuilder(pBuilder)
{
}

NodeDefinition::~NodeDefinition()
{
}

const std::string& NodeDefinition::getName() const
{
    return m_sName;
}

NodeBuilder NodeDefinition::getBuilder() const
{
    return m_pBuilder;
}

const ArgList& NodeDefinition::getDefaultArgs() const
{
    return m_Args;
}

const ChildMap& NodeDefinition::getChildren() const
{
    return m_Children;
}

NodeDefinition& NodeDefinition::extendDefinition(const NodeDefinition& Def)
{
    m_Args.copyArgsFrom(Def.m_Args);
    m_Children.insert(Def.m_Children.begin(), Def.m_Children.end());
    return *this;
}

NodeDefinition& NodeDefinition::addArg(const ArgBase& newArg)
{
    m_Args.setArg(newArg);
    return *this;
}

NodeDefinition& NodeDefinition::addChild(const NodeDefinition& Def)
{
    m_Children.insert(ChildMap::value_type(Def.getName(), Def));
    return *this;
}

NodeDefinition& NodeDefinition::addChildren(const ChildMap& Children)
{
    for(ChildMap::const_iterator otherIt = Children.begin(); otherIt != Children.end(); otherIt++)
    {
        ChildMap::const_iterator thisIt = m_Children.find(otherIt->first);
        if (thisIt == m_Children.end()) {
            m_Children.insert(*otherIt);
        }
        // TODO: if duplicate node defs differ in structure, throw exception
    }
    return *this;
}

}
