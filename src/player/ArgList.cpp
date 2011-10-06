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

#include "ArgList.h"

#include "Node.h"

#include "../base/Logger.h"
#include "../base/Exception.h"
#include "../base/StringHelper.h"
#include "../base/UTF8String.h"

#include <sstream>

using namespace std;

namespace avg {

ArgList::ArgList()
{
}

ArgList::ArgList(const ArgList& argTemplates, const xmlNodePtr xmlNode)
{
    copyArgsFrom(argTemplates);

    for (xmlAttrPtr prop = xmlNode->properties; prop; prop = prop->next)
    {
        string name = (char*)prop->name;
        string value = (char*)prop->children->content;
        setArgValue(name, value);
    }
}

ArgList::ArgList(const ArgList& argTemplates, const boost::python::dict& PyDict)
{
    // TODO: Check if all required args are being set.
    copyArgsFrom(argTemplates);
    boost::python::list keys = PyDict.keys();
    int nKeys = boost::python::len(keys);
    for (int i = 0; i < nKeys; i++)
    {
        boost::python::object keyObj = keys[i];
        boost::python::object valObj = PyDict[keyObj];
        
        boost::python::extract<string> keyStrProxy(keyObj);
        if (!keyStrProxy.check()) {
            throw Exception(AVG_ERR_INVALID_ARGS, "Argument name must be a string.");
        }
        string keyStr = keyStrProxy();

        setArgValue(keyStr, valObj);
    }
}

ArgList::~ArgList()
{
}

bool ArgList::hasArg(const std::string& sName) const
{
    ArgMap::const_iterator it = m_Args.find(sName);
    return (it != m_Args.end() && !(it->second->isDefault()));
}

const ArgBasePtr ArgList::getArg(const string& sName) const
{
    ArgMap::const_iterator valIt = m_Args.find(sName);
    if (valIt == m_Args.end()) {
        // TODO: The error message should mention line number and node type.
        throw Exception(AVG_ERR_INVALID_ARGS, string("Argument ")+sName+" is not valid.");
    }
    return valIt->second;
}

void ArgList::getOverlayedArgVal(DPoint* pResult, const string& sName, 
        const string& sOverlay1, const string& sOverlay2, const string& sID) const
{
    if (hasArg(sName)) {
        if (hasArg(sOverlay1) || hasArg(sOverlay2)) {
            throw (Exception(AVG_ERR_INVALID_ARGS,
                    string("Duplicate node arguments (")+sName+" and "+
                    sOverlay1+","+sOverlay2+") for node '"+sID+"'"));
        }
        *pResult = getArgVal<DPoint>(sName);
    }
}

const ArgMap& ArgList::getArgMap() const
{
    return m_Args;
}

void ArgList::setArg(const ArgBase& newArg)
{
    m_Args.insert(ArgMap::value_type(newArg.getName(), ArgBasePtr(newArg.createCopy())));
}

void ArgList::setArgs(const ArgList& args)
{
    for (ArgMap::const_iterator it = args.m_Args.begin(); it != args.m_Args.end(); it++) {
        m_Args.insert(*it);
    }
}
    
void ArgList::setMembers(Node * pNode) const
{
    for (ArgMap::const_iterator it = m_Args.begin(); it != m_Args.end(); it++) {
        const ArgBasePtr pCurArg = it->second;
        pCurArg->setMember(pNode);
    }
    pNode->setArgs(*this);
}

template<class T>
void setArgValue(Arg<T>* pArg, const std::string & sName, 
        const boost::python::object& value)
{
    boost::python::extract<T> valProxy(value);
    if (!valProxy.check()) {
        string sTypeName = getFriendlyTypeName(pArg->getValue());
        throw Exception(AVG_ERR_INVALID_ARGS, "Type error in argument "+sName+": "
                +sTypeName+" expected.");
    }
    pArg->setValue(valProxy());
}

void ArgList::setArgValue(const std::string & sName, const boost::python::object& value)
{
    ArgBasePtr pArg = getArg(sName);
    Arg<string>* pStringArg = dynamic_cast<Arg<string>* >(&*pArg);
    Arg<UTF8String>* pUTF8StringArg = dynamic_cast<Arg<UTF8String>* >(&*pArg);
    Arg<int>* pIntArg = dynamic_cast<Arg<int>* >(&*pArg);
    Arg<double>* pDoubleArg = dynamic_cast<Arg<double>* >(&*pArg);
    Arg<float>* pFloatArg = dynamic_cast<Arg<float>* >(&*pArg);
    Arg<bool>* pBoolArg = dynamic_cast<Arg<bool>* >(&*pArg);
    Arg<DPoint>* pDPointArg = dynamic_cast<Arg<DPoint>* >(&*pArg);
    Arg<IntTriple>* pIntTripleArg = dynamic_cast<Arg<IntTriple>* >(&*pArg);
    Arg<DTriple>* pDTripleArg = dynamic_cast<Arg<DTriple>* >(&*pArg);
    Arg<vector<double> >* pDVectorArg = dynamic_cast<Arg<vector<double> >* >(&*pArg);
    Arg<vector<DPoint> >* pDPointVectorArg = 
            dynamic_cast<Arg<vector<DPoint> >* >(&*pArg);
    Arg<vector<IntTriple> >* pIntTripleVectorArg = 
            dynamic_cast<Arg<vector<IntTriple> >* >(&*pArg);
    if(pStringArg) {
        avg::setArgValue(pStringArg, sName, value);
    } else if (pUTF8StringArg) {
        avg::setArgValue(pUTF8StringArg, sName, value);
    } else if (pIntArg) {
        avg::setArgValue(pIntArg, sName, value);
    } else if (pDoubleArg) {
        avg::setArgValue(pDoubleArg, sName, value);
    } else if (pFloatArg) {
        avg::setArgValue(pFloatArg, sName, value);
    } else if (pBoolArg) {
        avg::setArgValue(pBoolArg, sName, value);
    } else if (pDPointArg) {
        avg::setArgValue(pDPointArg, sName, value);
    } else if (pDVectorArg) {
        avg::setArgValue(pDVectorArg, sName, value);
    } else if (pDPointVectorArg) {
        avg::setArgValue(pDPointVectorArg, sName, value);
    } else if (pIntTripleArg) {
        avg::setArgValue(pIntTripleArg, sName, value);
    } else if (pDTripleArg) {
        avg::setArgValue(pDTripleArg, sName, value);
    } else if (pIntTripleVectorArg) {
        avg::setArgValue(pIntTripleVectorArg, sName, value);
    } else {
        AVG_ASSERT(false);
    }
}

void ArgList::setArgValue(const std::string & sName, const std::string & sValue)
{
    ArgBasePtr pArg = getArg(sName);
    Arg<string>* pStringArg = dynamic_cast<Arg<string>* >(&*pArg);
    Arg<UTF8String>* pUTF8StringArg = dynamic_cast<Arg<UTF8String>* >(&*pArg);
    Arg<int>* pIntArg = dynamic_cast<Arg<int>* >(&*pArg);
    Arg<double>* pDoubleArg = dynamic_cast<Arg<double>* >(&*pArg);
    Arg<float>* pFloatArg = dynamic_cast<Arg<float>* >(&*pArg);
    Arg<bool>* pBoolArg = dynamic_cast<Arg<bool>* >(&*pArg);
    Arg<DPoint>* pDPointArg = dynamic_cast<Arg<DPoint>* >(&*pArg);
    Arg<IntTriple>* pIntTripleArg = dynamic_cast<Arg<IntTriple>* >(&*pArg);
    Arg<vector<double> >* pDVectorArg = dynamic_cast<Arg<vector<double> >* >(&*pArg);
    Arg<vector<DPoint> >* pDPointVectorArg = 
            dynamic_cast<Arg<vector<DPoint> >* >(&*pArg);
    Arg<vector<IntTriple> >* pIntTripleVectorArg = 
            dynamic_cast<Arg<vector<IntTriple> >* >(&*pArg);

    if (pStringArg) {
        pStringArg->setValue(sValue);
    } else if (pUTF8StringArg) {
        pUTF8StringArg->setValue(sValue);
    } else if (pIntArg) {
        pIntArg->setValue(stringToInt(sValue));
    } else if (pDoubleArg) {
        pDoubleArg->setValue(stringToDouble(sValue));
    } else if (pFloatArg) {
        pFloatArg->setValue(float(stringToDouble(sValue)));
    } else if (pBoolArg) {
        pBoolArg->setValue(stringToBool(sValue));
    } else if (pDPointArg) {
        pDPointArg->setValue(stringToDPoint(sValue));
    } else if (pIntTripleArg) {
        pIntTripleArg->setValue(stringToIntTriple(sValue));
    } else if (pDVectorArg) {
        vector<double> v;
        fromString(sValue, v);
        pDVectorArg->setValue(v);
    } else if (pDPointVectorArg) {
        vector<DPoint> v;
        fromString(sValue, v);
        pDPointVectorArg->setValue(v);
    } else if (pIntTripleVectorArg) {
        vector<IntTriple> v;
        fromString(sValue, v);
        pIntTripleVectorArg->setValue(v);
    } else {
        AVG_ASSERT(false);
    }   
}

void ArgList::copyArgsFrom(const ArgList& argTemplates)
{
    for (ArgMap::const_iterator it = argTemplates.m_Args.begin();
            it != argTemplates.m_Args.end(); it++)
    {
        string sKey = it->first;
        ArgBasePtr pArg = ArgBasePtr(it->second->createCopy());
        m_Args[sKey] = pArg;
    }
}

}

