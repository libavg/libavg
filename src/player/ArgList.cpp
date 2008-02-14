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

#include "ArgList.h"

#include "../base/Logger.h"
#include "../base/Exception.h"

#include <sstream>

using namespace std;

namespace avg {

ArgList::ArgList()
{
}

ArgList::ArgList(const xmlNodePtr xmlNode)
{
    for(xmlAttrPtr prop = xmlNode->properties; prop; prop = prop->next)
    {
        string name = (char*)prop->name;
        string value = (char*)prop->children->content;
        m_Args.insert(ArgMap::value_type(name, Arg(name, value)));
    }
}

ArgList::ArgList(const boost::python::dict& PyDict)
{
    boost::python::list keys = PyDict.keys();
    int nKeys = boost::python::len(keys);
    for(int i = 0; i < nKeys; i++)
    {
        boost::python::object keyObj = keys[i];
        boost::python::object valObj = PyDict[keyObj];
        
        boost::python::extract<string> keyStrProxy(keyObj);
        boost::python::extract<string> valStrProxy(valObj);
        
        if (!keyStrProxy.check()) {
            AVG_TRACE(Logger::WARNING, "Invalid argument name type (must be str)");
            continue;
        }
        if (!valStrProxy.check()) {
            AVG_TRACE(Logger::WARNING, "Invalid argument value type (must be str)");
            continue;
        }
        
        string keyStr = keyStrProxy();
        string valStr = valStrProxy();
        
        m_Args.insert(ArgMap::value_type(keyStr, Arg(keyStr, valStr)));
    }
}

ArgList::~ArgList()
{
}

const Arg& ArgList::getArg(const string& Name) const
{
    ArgMap::const_iterator valIt = m_Args.find(Name);
    if (valIt == m_Args.end()) {
        throw Exception(AVG_ERR_NO_ARG, string("No arg '")+Name+"'");
    }
    return valIt->second;
}

int ArgList::getIntArg(const string& Name) const
{
    return getArg(Name).toInt();
}

double ArgList::getDoubleArg(const string& Name) const
{
    return getArg(Name).toDouble();
}

bool ArgList::getBoolArg(const string& Name) const
{
    return getArg(Name).toBool();
}

string ArgList::getStringArg(const string& Name) const
{
    return getArg(Name).toString();
}

const ArgMap& ArgList::getArgMap() const
{
    return m_Args;
}

void ArgList::setArg(const string& Name, int Value, bool bRequired)
{
    stringstream ss;
    ss << Value;
    setArg(Name, ss, bRequired);
}

void ArgList::setArg(const string& Name, double Value, bool bRequired)
{
    stringstream ss;
    ss << Value;
    setArg(Name, ss, bRequired);
}

void ArgList::setArg(const string& Name, bool Value, bool bRequired)
{
    setArg(Name, (Value ? "true" : "false"), bRequired);
}

void ArgList::setArg(const string& Name, const string& Value, bool bRequired)
{
    m_Args.insert(ArgMap::value_type(Name, Arg(Name, Value, bRequired)));
}

void ArgList::setArgs(const ArgList& Args)
{
    for(ArgMap::const_iterator it = Args.m_Args.begin(); it != Args.m_Args.end(); it++)
    {
        m_Args.insert(*it);
    }
}

ArgList ArgList::operator+(const ArgList& OtherArgs) const
{
    ArgList newArgs(*this);
    for(ArgMap::const_iterator it = OtherArgs.m_Args.begin(); it != OtherArgs.m_Args.end(); it++)
    {
        ArgMap::const_iterator it2 = newArgs.m_Args.find(it->first);
        if (it2 == newArgs.m_Args.end()) {
            newArgs.m_Args.insert(*it);
        }
    }
    return newArgs;
}

}

