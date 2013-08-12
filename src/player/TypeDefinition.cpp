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

#include "TypeDefinition.h"

#include "../base/Logger.h"

using namespace std;

namespace avg {

TypeDefinition::TypeDefinition() :
      m_pBuilder(0)
{
}

TypeDefinition::TypeDefinition(const string& sName, const string& sBaseName,
        ObjectBuilder pBuilder)
    : m_sName(sName),
      m_pBuilder(pBuilder)
{
    if (sBaseName != "") {
        TypeDefinition baseDef = TypeRegistry::get()->getTypeDef(sBaseName);
        m_Args.copyArgsFrom(baseDef.m_Args);
        m_sChildren = baseDef.m_sChildren;
    }
}

TypeDefinition::~TypeDefinition()
{
}

const std::string& TypeDefinition::getName() const
{
    return m_sName;
}

ObjectBuilder TypeDefinition::getBuilder() const
{
    return m_pBuilder;
}

const ArgList& TypeDefinition::getDefaultArgs() const
{
    return m_Args;
}

const string& TypeDefinition::getDTDElements() const
{
    return m_sDTDElements;
}

string TypeDefinition::getDTDChildrenString() const
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

bool TypeDefinition::isChildAllowed(const string& sChild) const
{
    for (unsigned i=0; i<m_sChildren.size(); ++i) {
        if (m_sChildren[i] == sChild) {
            return true;
        }
    }
    return false;
}

bool TypeDefinition::hasChildren() const
{
    return !m_sChildren.empty();
}

bool TypeDefinition::isAbstract() const
{
    return m_pBuilder == 0;
}

TypeDefinition& TypeDefinition::addArg(const ArgBase& newArg)
{
    m_Args.setArg(newArg);
    return *this;
}

TypeDefinition& TypeDefinition::addDTDElements(const string& s)
{
    m_sDTDElements = s;
    return *this;
}

TypeDefinition& TypeDefinition::addChildren(const vector<string>& sChildren)
{
    m_sChildren.insert(m_sChildren.end(), sChildren.begin(), sChildren.end());
    return *this;
}

TypeDefinition& TypeDefinition::addChild(const string& sChild)
{
    m_sChildren.push_back(sChild);
    return *this;
}

}
