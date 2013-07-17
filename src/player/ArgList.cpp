//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2013 Ulrich von Zadow
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

#include "ExportedObject.h"
#include "FontStyle.h"

#include "../base/Logger.h"
#include "../base/Exception.h"
#include "../base/StringHelper.h"
#include "../base/UTF8String.h"

#include <sstream>

using namespace std;

namespace avg {

typedef std::vector<std::vector<glm::vec2> > CollVec2Vector;

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

ArgList::ArgList(const ArgList& argTemplates, const py::dict& PyDict)
{
    // TODO: Check if all required args are being set.
    copyArgsFrom(argTemplates);
    py::list keys = PyDict.keys();
    int nKeys = py::len(keys);
    for (int i = 0; i < nKeys; i++)
    {
        py::object keyObj = keys[i];
        py::object valObj = PyDict[keyObj];
        
        py::extract<string> keyStrProxy(keyObj);
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

void ArgList::getOverlayedArgVal(glm::vec2* pResult, const string& sName, 
        const string& sOverlay1, const string& sOverlay2, const string& sID) const
{
    if (hasArg(sName)) {
        if (hasArg(sOverlay1) || hasArg(sOverlay2)) {
            throw (Exception(AVG_ERR_INVALID_ARGS,
                    string("Duplicate node arguments (")+sName+" and "+
                    sOverlay1+","+sOverlay2+") for node '"+sID+"'"));
        }
        *pResult = getArgVal<glm::vec2>(sName);
    }
}

const ArgMap& ArgList::getArgMap() const
{
    return m_Args;
}

void ArgList::setArg(const ArgBase& newArg)
{
    m_Args[newArg.getName()] =  ArgBasePtr(newArg.createCopy());
}

void ArgList::setArgs(const ArgList& args)
{
    for (ArgMap::const_iterator it = args.m_Args.begin(); it != args.m_Args.end(); it++) {
        m_Args.insert(*it);
    }
}
    
void ArgList::setMembers(ExportedObject * pObj) const
{
    for (ArgMap::const_iterator it = m_Args.begin(); it != m_Args.end(); it++) {
        const ArgBasePtr pCurArg = it->second;
        pCurArg->setMember(pObj);
    }
    pObj->setArgs(*this);
}

template<class T>
void setArgValue(Arg<T>* pArg, const std::string & sName, const py::object& value)
{
    py::extract<T> valProxy(value);
    if (!valProxy.check()) {
        string sTypeName = getFriendlyTypeName(pArg->getValue());
        throw Exception(AVG_ERR_INVALID_ARGS, "Type error in argument "+sName+": "
                +sTypeName+" expected.");
    }
    pArg->setValue(valProxy());
}

void ArgList::setArgValue(const std::string & sName, const py::object& value)
{
    ArgBasePtr pArg = getArg(sName);
    Arg<string>* pStringArg = dynamic_cast<Arg<string>* >(&*pArg);
    Arg<UTF8String>* pUTF8StringArg = dynamic_cast<Arg<UTF8String>* >(&*pArg);
    Arg<int>* pIntArg = dynamic_cast<Arg<int>* >(&*pArg);
    Arg<float>* pFloatArg = dynamic_cast<Arg<float>* >(&*pArg);
    Arg<bool>* pBoolArg = dynamic_cast<Arg<bool>* >(&*pArg);
    Arg<glm::vec2>* pVec2Arg = dynamic_cast<Arg<glm::vec2>* >(&*pArg);
    Arg<glm::vec3>* pVec3Arg = dynamic_cast<Arg<glm::vec3>* >(&*pArg);
    Arg<glm::ivec3>* pIVec3Arg = dynamic_cast<Arg<glm::ivec3>* >(&*pArg);
    Arg<vector<float> >* pFVectorArg = dynamic_cast<Arg<vector<float> >* >(&*pArg);
    Arg<vector<int> >* pIVectorArg = dynamic_cast<Arg<vector<int> >* >(&*pArg);
    Arg<vector<glm::vec2> >* pVec2VectorArg = 
            dynamic_cast<Arg<vector<glm::vec2> >* >(&*pArg);
    Arg<vector<glm::ivec3> >* pIVec3VectorArg = 
            dynamic_cast<Arg<vector<glm::ivec3> >* >(&*pArg);
    Arg<CollVec2Vector>* pCollVec2VectorArg =
            dynamic_cast<Arg<CollVec2Vector>* >(&*pArg);
    Arg<FontStyle>* pFontStyleArg = dynamic_cast<Arg<FontStyle>* >(&*pArg);
    Arg<FontStylePtr>* pFontStylePtrArg = dynamic_cast<Arg<FontStylePtr>* >(&*pArg);
    if(pStringArg) {
        avg::setArgValue(pStringArg, sName, value);
    } else if (pUTF8StringArg) {
        avg::setArgValue(pUTF8StringArg, sName, value);
    } else if (pIntArg) {
        avg::setArgValue(pIntArg, sName, value);
    } else if (pFloatArg) {
        avg::setArgValue(pFloatArg, sName, value);
    } else if (pBoolArg) {
        avg::setArgValue(pBoolArg, sName, value);
    } else if (pVec2Arg) {
        avg::setArgValue(pVec2Arg, sName, value);
    } else if (pVec3Arg) {
        avg::setArgValue(pVec3Arg, sName, value);
    } else if (pIVec3Arg) {
        avg::setArgValue(pIVec3Arg, sName, value);
    } else if (pFVectorArg) {
        avg::setArgValue(pFVectorArg, sName, value);
    } else if (pIVectorArg) {
        avg::setArgValue(pIVectorArg, sName, value);
    } else if (pVec2VectorArg) {
        avg::setArgValue(pVec2VectorArg, sName, value);
    } else if (pIVec3VectorArg) {
        avg::setArgValue(pIVec3VectorArg, sName, value);
    } else if (pCollVec2VectorArg) {
        avg::setArgValue(pCollVec2VectorArg, sName, value);
    } else if (pFontStyleArg) {
        avg::setArgValue(pFontStyleArg, sName, value);
    } else if (pFontStylePtrArg) {
        avg::setArgValue(pFontStylePtrArg, sName, value);
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
    Arg<float>* pFloatArg = dynamic_cast<Arg<float>* >(&*pArg);
    Arg<bool>* pBoolArg = dynamic_cast<Arg<bool>* >(&*pArg);
    Arg<glm::vec2>* pVec2Arg = dynamic_cast<Arg<glm::vec2>* >(&*pArg);
    Arg<glm::vec3>* pVec3Arg = dynamic_cast<Arg<glm::vec3>* >(&*pArg);
    Arg<glm::ivec3>* pIVec3Arg = dynamic_cast<Arg<glm::ivec3>* >(&*pArg);
    Arg<vector<float> >* pFVectorArg = dynamic_cast<Arg<vector<float> >* >(&*pArg);
    Arg<vector<int> >* pIVectorArg = dynamic_cast<Arg<vector<int> >* >(&*pArg);
    Arg<vector<glm::vec2> >* pVec2VectorArg = 
            dynamic_cast<Arg<vector<glm::vec2> >* >(&*pArg);
    Arg<vector<glm::ivec3> >* pIVec3VectorArg = 
            dynamic_cast<Arg<vector<glm::ivec3> >* >(&*pArg);
    Arg<CollVec2Vector>* pCollVec2VectorArg =
            dynamic_cast<Arg<CollVec2Vector>* >(&*pArg);
    if (pStringArg) {
        pStringArg->setValue(sValue);
    } else if (pUTF8StringArg) {
        pUTF8StringArg->setValue(sValue);
    } else if (pIntArg) {
        pIntArg->setValue(stringToInt(sValue));
    } else if (pFloatArg) {
        pFloatArg->setValue(stringToFloat(sValue));
    } else if (pBoolArg) {
        pBoolArg->setValue(stringToBool(sValue));
    } else if (pVec2Arg) {
        pVec2Arg->setValue(stringToVec2(sValue));
    } else if (pVec3Arg) {
        pVec3Arg->setValue(stringToVec3(sValue));
    } else if (pIVec3Arg) {
        pIVec3Arg->setValue(stringToIVec3(sValue));
    } else if (pFVectorArg) {
        vector<float> v;
        fromString(sValue, v);
        pFVectorArg->setValue(v);
    } else if (pIVectorArg) {
        vector<int> v;
        fromString(sValue, v);
        pIVectorArg->setValue(v);
    } else if (pVec2VectorArg) {
        vector<glm::vec2> v;
        fromString(sValue, v);
        pVec2VectorArg->setValue(v);
    } else if (pIVec3VectorArg) {
        vector<glm::ivec3> v;
        fromString(sValue, v);
        pIVec3VectorArg->setValue(v);
    } else if (pCollVec2VectorArg) {
        CollVec2Vector v;
        fromString(sValue, v);
        pCollVec2VectorArg->setValue(v);
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

