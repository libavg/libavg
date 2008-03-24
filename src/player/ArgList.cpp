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

#include "Node.h"

#include "../base/Logger.h"
#include "../base/Exception.h"

#include <sstream>

using namespace std;

namespace avg {

ArgList::ArgList()
{
}

ArgList::ArgList(const ArgList& ArgTemplates, const xmlNodePtr xmlNode)
{
    copyArgsFrom(ArgTemplates);

    for(xmlAttrPtr prop = xmlNode->properties; prop; prop = prop->next)
    {
        string name = (char*)prop->name;
        string value = (char*)prop->children->content;
        setArgValue(name, value);
    }
}

ArgList::ArgList(const ArgList& ArgTemplates, const boost::python::dict& PyDict)
{
    // TODO: Check if all required args are being set.
    copyArgsFrom(ArgTemplates);
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
       
        setArgValue(keyStr, valStr);
    }
}

ArgList::~ArgList()
{
}

const ArgBasePtr ArgList::getArg(const string& Name) const
{
    ArgMap::const_iterator valIt = m_Args.find(Name);
    if (valIt == m_Args.end()) {
        throw Exception(AVG_ERR_NO_ARG, string("No arg '")+Name+"'");
    }
    return valIt->second;
}

const ArgMap& ArgList::getArgMap() const
{
    return m_Args;
}

void ArgList::setArg(const ArgBase& newArg)
{
    m_Args.insert(ArgMap::value_type(newArg.getName(), ArgBasePtr(newArg.createCopy())));
}

void ArgList::setArgs(const ArgList& Args)
{
    for(ArgMap::const_iterator it = Args.m_Args.begin(); 
            it != Args.m_Args.end(); it++)
    {
        m_Args.insert(*it);
    }
}
    
void ArgList::setMembers(Node * pNode) const
{
    for(ArgMap::const_iterator it = m_Args.begin(); it != m_Args.end(); it++)
    {
        const ArgBasePtr pCurArg = it->second;
        pCurArg->setMember(pNode);
    }
    pNode->setArgs(*this);
}

void ArgList::setArgValue(const std::string & sName, const std::string & sValue)
{
    ArgMap::iterator pos = m_Args.find(sName);
    if (pos == m_Args.end()) {
        // TODO: The error message should mention line number and node type.
        throw Exception(AVG_ERR_INVALID_ARGS, string("Argument ")+sName+" is not valid.");
    }
    ArgBasePtr pArg = pos->second;
    Arg<string>* pStringArg = dynamic_cast<Arg<string>* >(&*pArg);
    Arg<int>* pIntArg = dynamic_cast<Arg<int>* >(&*pArg);
    Arg<double>* pDoubleArg = dynamic_cast<Arg<double>* >(&*pArg);
    Arg<bool>* pBoolArg = dynamic_cast<Arg<bool>* >(&*pArg);
    if (pStringArg) {
        pStringArg->setValue(sValue);
    } else if (pIntArg) {
        char * errStr;
        const char * valStr = sValue.c_str();
        int ret = strtol(valStr, &errStr, 10);
        if (ret == 0 && errStr == valStr) {
            throw Exception(AVG_ERR_NO_ARG, 
                    string("Error in conversion of '")+sValue+"' to int");
        }
        pIntArg->setValue(ret);
    } else if (pDoubleArg) {
        char * errStr;
        const char * valStr = sValue.c_str();
        double ret = strtod(valStr, &errStr);
        if (ret == 0 && errStr == valStr) {
            throw Exception(AVG_ERR_NO_ARG, 
                    string("Error in conversion of '")+sValue+"' to double");
        }
        pDoubleArg->setValue(ret); 
    } else if (pBoolArg) {
        pBoolArg->setValue(sValue == "True" || sValue == "true" || sValue == "1");
    }
}

void ArgList::copyArgsFrom(const ArgList& ArgTemplates)
{
    for(ArgMap::const_iterator it = ArgTemplates.m_Args.begin();
            it != ArgTemplates.m_Args.end(); it++)
    {
        string sKey = it->first;
        ArgBasePtr pArg = ArgBasePtr(it->second->createCopy());
        m_Args[sKey] = pArg;
    }
}

}

