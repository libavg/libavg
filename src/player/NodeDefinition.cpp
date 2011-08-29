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

#include "NodeDefinition.h"

#include "../base/Logger.h"

using namespace std;

namespace avg {

NodeDefinition::NodeDefinition() :
      m_pBuilder(0)
{
}

NodeDefinition::NodeDefinition(const string& sName, NodeBuilder pBuilder)
    : m_sName(sName),
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

const string& NodeDefinition::getDTDElements() const
{
    return m_sDTDElements;
}

string NodeDefinition::getDTDChildrenString() const
{
    if (m_sChildren.empty()) {
        return "EMPTY";
    } else {
        string sChildren = "(";

        for (unsigned i=0; i<m_sChildren.size()-1; ++i) {
            sChildren += m_sChildren[i]+"|";
        }
        sChildren += m_sChildren[m_sChildren.size()-1]+")*";
        return sChildren;
    }
}

bool NodeDefinition::isChildAllowed(const string& sChild) const
{
    for (unsigned i=0; i<m_sChildren.size(); ++i) {
        if (m_sChildren[i] == sChild) {
            return true;
        }
    }
    return false;
}

bool NodeDefinition::hasChildren() const
{
    return !m_sChildren.empty();
}

NodeDefinition& NodeDefinition::extendDefinition(const NodeDefinition& Def)
{
    m_Args.copyArgsFrom(Def.m_Args);
    m_sChildren = Def.m_sChildren;
    return *this;
}

NodeDefinition& NodeDefinition::addArg(const ArgBase& newArg)
{
    m_Args.setArg(newArg);
    return *this;
}

NodeDefinition& NodeDefinition::addDTDElements(const string& s)
{
    m_sDTDElements = s;
    return *this;
}

NodeDefinition& NodeDefinition::addChildren(const vector<string>& sChildren)
{
    m_sChildren.insert(m_sChildren.end(), sChildren.begin(), sChildren.end());
    return *this;
}

}
