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

#include "ExportedObject.h"
#include "TypeDefinition.h"
#include "Arg.h"

#include "../base/Exception.h"
#include "../base/Logger.h"
#include "../base/ObjectCounter.h"

#include <string>

using namespace std;

namespace avg {

ExportedObject::ExportedObject()
    : m_pSelf(0)
{
    ObjectCounter::get()->incRef(&typeid(*this));
}

ExportedObject::ExportedObject(const ExportedObject& other)
    : m_pSelf(0)
{
    AVG_ASSERT(!other.m_pSelf);
    m_pDefinition = other.m_pDefinition;
    ObjectCounter::get()->incRef(&typeid(*this));
}

ExportedObject::~ExportedObject()
{
    ObjectCounter::get()->decRef(&typeid(*this));
}

void ExportedObject::registerInstance(PyObject* pSelf)
{
    m_pSelf = pSelf;
}

ExportedObjectPtr ExportedObject::getSharedThis()
{
    // Just using shared_from_this causes strange behaviour when derived classes
    // are written in python: The pointer returned by shared_from_this doesn't know
    // about the python part of the object and cuts it off. Because of this, we remember
    // a pointer to the python object in m_pSelf and use that to create a functioning
    // and complete ExportedObjectPtr if there is a python derived class.
    if (m_pSelf) {
        return py::extract<ExportedObjectPtr>(m_pSelf);
    } else {
        return shared_from_this();
    }
}

void ExportedObject::setTypeInfo(const TypeDefinition * pDefinition)
{
    m_pDefinition = pDefinition;
}

const TypeDefinition* ExportedObject::getDefinition() const
{
    return m_pDefinition;
}

string ExportedObject::getTypeStr() const
{
    return m_pDefinition->getName();
}

bool ExportedObject::operator ==(const ExportedObject& other) const
{
    return this == &other;
}

bool ExportedObject::operator !=(const ExportedObject& other) const
{
    return this != &other;
}

long ExportedObject::getHash() const
{
    return long(this);
}


}
