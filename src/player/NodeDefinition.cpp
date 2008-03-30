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
      m_pBuilder(pBuilder),
      m_bIsGroupNode(false)
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

bool NodeDefinition::isGroupNode() const
{
    return m_bIsGroupNode;
}

const string& NodeDefinition::getDTDElements() const
{
    return m_sDTDElements;
}

const string& NodeDefinition::getChildren() const
{
    return m_sChildren;
}


NodeDefinition& NodeDefinition::extendDefinition(const NodeDefinition& Def)
{
    m_Args.copyArgsFrom(Def.m_Args);
    return *this;
}

NodeDefinition& NodeDefinition::addArg(const ArgBase& newArg)
{
    m_Args.setArg(newArg);
    return *this;
}

NodeDefinition& NodeDefinition::setGroupNode()
{
    m_bIsGroupNode = true;
    return *this;
}
    
NodeDefinition& NodeDefinition::addDTDElements(const string& s)
{
    m_sDTDElements = s;
    return *this;
}

NodeDefinition& NodeDefinition::addChildren(const string& s)
{
    m_sChildren = s;
    return *this;
}

}
