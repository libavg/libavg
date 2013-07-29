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

#include "ConfigMgr.h"
#include "Logger.h"
#include "Exception.h"
#include "OSHelper.h"

#include <libxml/xmlmemory.h>

#include <iostream>
#include <stdlib.h>
#include <errno.h>

#ifndef _WIN32
#include <unistd.h>
#else
#include  <io.h>
#endif

using namespace std;

namespace avg {

ConfigOption::ConfigOption(const string& sName, const string& sValue)
    : m_sName(sName),
      m_sValue(sValue)
{
}

ConfigMgr* ConfigMgr::m_pGlobalConfigMgr = 0;

void deleteConfigMgr()
{
    delete ConfigMgr::m_pGlobalConfigMgr;
    ConfigMgr::m_pGlobalConfigMgr = 0;
}

ConfigMgr* ConfigMgr::get()
{
    if (!m_pGlobalConfigMgr) {
        m_pGlobalConfigMgr = new ConfigMgr;
        atexit(deleteConfigMgr);
    }
    return m_pGlobalConfigMgr;
}

ConfigMgr::ConfigMgr()
{
    addSubsys("scr");
    addOption("scr", "gles", "false");
    addOption("scr", "bpp", "24");
    addOption("scr", "fullscreen", "false");
    addOption("scr", "windowwidth", "0");
    addOption("scr", "windowheight", "0");
    addOption("scr", "dotspermm", "0");
    addOption("scr", "usepow2textures", "false");
    addOption("scr", "usepixelbuffers", "true");
    addOption("scr", "multisamplesamples", "8");
    addOption("scr", "shaderusage", "auto");
    addOption("scr", "gamma", "-1,-1,-1");
    addOption("scr", "vsyncmode", "auto");
    addOption("scr", "videoaccel", "true");
    
    addSubsys("aud");
    addOption("aud", "channels", "2");
    addOption("aud", "samplerate", "44100");
    addOption("aud", "outputbuffersamples", "1024");

    addSubsys("gesture");
    addOption("gesture", "maxtapdist", "15");
    addOption("gesture", "maxdoubletaptime", "300");
    addOption("gesture", "minswipedist", "50");
    addOption("gesture", "swipedirectiontolerance", "0.393"); // pi/8
    addOption("gesture", "maxswipecontactdist", "100");
    addOption("gesture", "holddelay", "500");
    addOption("gesture", "mindragdist", "5");
    addOption("gesture", "filtermincutoff", "0.1");
    addOption("gesture", "filterbeta", "0.03");
    addOption("gesture", "friction", "-1");

    addSubsys("touch");
    addOption("touch", "area", "0, 0");
    addOption("touch", "offset", "0, 0");

    m_sFName = "avgrc";
    loadFile(getGlobalConfigDir()+m_sFName);
    char * pHome = getenv("HOME");
    if (pHome) {
        loadFile(string(pHome)+"/."+m_sFName);
    }
}

void ConfigMgr::addSubsys(const string& sName)
{
    m_SubsysOptionMap[sName] = ConfigOptionVector();
}

void ConfigMgr::addOption(const string& sSubsys, const string& sName,
        const std::string& sDefault)
{
    ConfigOptionVector& Subsys = m_SubsysOptionMap[sSubsys];
    Subsys.push_back(ConfigOption(sName, sDefault));
}

const ConfigOptionVector* ConfigMgr::getOptions(const string& sSubsys) const
{
    SubsysOptionMap::const_iterator it = m_SubsysOptionMap.find(sSubsys);
    if (it == m_SubsysOptionMap.end()) {
        return 0;
    } else {
        return &(*it).second;
    }
}

const string* ConfigMgr::getOption(const string& sSubsys, 
        const string& sName) const
{
    const ConfigOptionVector* pOptionVector = getOptions(sSubsys);
    if (!pOptionVector) {
        return 0;
    } else {
        for (unsigned int i=0; i<pOptionVector->size(); i++) {
            if ((*pOptionVector)[i].m_sName == sName) {
                return &(*pOptionVector)[i].m_sValue;
            }
        }
        return 0;
    }
}

bool ConfigMgr::getBoolOption(const string& sSubsys, 
            const string& sName, bool bDefault) const
{
    const string * psOption = getOption(sSubsys, sName);
    if (psOption == 0) {
        return bDefault;
    }
    if (*psOption == "true") {
        return true;
    } else if (*psOption == "false") {
        return false;
    } else {
        AVG_LOG_ERROR(m_sFName << ": Unrecognized value for option " << sName << ": " 
                << *psOption << ". Must be true or false. Aborting.");
        exit(-1);
    }
}

int ConfigMgr::getIntOption(const string& sSubsys, 
        const string& sName, int Default) const
{
    errno = 0;
    const string * psOption = getOption(sSubsys, sName);
    if (psOption == 0) {
        return Default;
    }
    int Result = strtol(psOption->c_str(), 0, 10);
    int rc = errno;
    if (rc == EINVAL || rc == ERANGE) {
        AVG_LOG_ERROR(m_sFName << ": Unrecognized value for option "<<sName<<": " 
                << *psOption << ". Must be an integer. Aborting.");
        exit(-1);
    }
    return Result;
}

void ConfigMgr::getGammaOption(const string& sSubsys, 
            const string& sName, float* Val) const
{
    const string * psOption = getOption(sSubsys, sName);
    if (psOption == 0) {
        return;
    }
    int rc = sscanf(psOption->c_str(), "%f,%f,%f", Val, Val+1, Val+2);
    if (rc < 3) {
        AVG_LOG_ERROR(m_sFName << ": Unrecognized value for option "<<sName<<": " 
                << *psOption << ". Must be three comma-separated numbers. Aborting.");
        exit(-1);
    }
}

glm::vec2 ConfigMgr::getSizeOption(const string& sSubsys,
        const string& sName) const
{
    const string * psOption = getOption(sSubsys, sName);
    if (psOption == 0) {
        return glm::vec2(0, 0);
    }
    float val[2];
    int rc = sscanf(psOption->c_str(), "%f,%f", val, val+1);
    if (rc < 2) {
        AVG_LOG_ERROR(m_sFName << ": Unrecognized value for option " << sName << ": "
                << *psOption << ". Must be 2 comma-separated numbers(x, y). Aborting.");
        exit(-1);
    }
    return glm::vec2(val[0], val[1]);
}

void ConfigMgr::getStringOption(const string& sSubsys, 
        const string& sName, const string& sDefault, string& sVal) const
{
    const string * psOption = getOption(sSubsys, sName);
    if (psOption == 0) {
        sVal = sDefault;
    } else {
        sVal = *psOption;
    }
}


bool ConfigMgr::loadFile(const std::string& sPath)
{
    string sSubsys;
    try {
#ifndef _WIN32
        // I don't think read permissions on config files are an issue under windows.
        int err = access(sPath.c_str(), R_OK);
        if (err == -1) {
            if (errno == EACCES) {
                AVG_LOG_WARNING(sPath+
                        ": File exists, but process doesn't have read permissions!");
            }
            return false;
        }
#else
        // but this actually prevents ugly XML parsing errors when file does not exist
        // and cygwin is used
        int err = _access(sPath.c_str(), 0);
        if (err == -1) {
            return false;
        }
#endif
        xmlDocPtr doc;
        doc = xmlParseFile(sPath.c_str());
        if (!doc) {
            throw Exception(AVG_ERR_XML_VALID, "Error parsing "+sPath
                    +". File is not well-formed.");
        }
        xmlNodePtr pRoot = xmlDocGetRootElement(doc);
        if (xmlStrcmp(pRoot->name, (const xmlChar *)(m_sFName.c_str()))) {
            AVG_LOG_ERROR(sPath+": Root node must be <"+m_sFName+">, found " 
                    << pRoot->name << ". Aborting.");
            exit(255);
        }
        xmlNodePtr pSubsysNode = pRoot->xmlChildrenNode;
        while (pSubsysNode) {
            if (xmlStrcmp(pSubsysNode->name, (const xmlChar *)"text") &&
                    xmlStrcmp(pSubsysNode->name, (const xmlChar *)"comment"))
            {
                sSubsys = ((const char *)pSubsysNode->name);
                xmlNodePtr pOptionNode = pSubsysNode->xmlChildrenNode;
                if (!pOptionNode) {
                    AVG_LOG_ERROR(sPath << ": Option " << sSubsys 
                            << " has no value. Ignoring.");
                } else {
                    ConfigOptionVector& CurSubsys = getSubsys(sSubsys);
                    while (pOptionNode) {
                        if (xmlStrcmp(pOptionNode->name, (const xmlChar *)"text") &&
                            xmlStrcmp(pOptionNode->name, (const xmlChar *)"comment"))
                        {
                            setOption(CurSubsys, doc, pOptionNode);
                        }
                        pOptionNode = pOptionNode->next;
                    }
                }
            }
            pSubsysNode = pSubsysNode->next;
        }
        xmlFreeDoc(doc);
    } catch (Exception& e) {
        switch (e.getCode()) {
            case AVG_ERR_OPTION_SUBSYS_UNKNOWN:
                AVG_LOG_ERROR("While parsing " << sPath << ": Option group " <<
                        e.getStr() << " unknown. Aborting.");
                exit(255);
            case AVG_ERR_OPTION_UNKNOWN: 
                AVG_LOG_ERROR("While parsing " << sPath  << ": Option " << sSubsys <<
                        ":" << e.getStr() << " unknown. Aborting.");
                exit(255);
            default:
                throw;
        }
    }
    return true;
}

ConfigOptionVector& ConfigMgr::getSubsys(const string& sName)
{
    SubsysOptionMap::iterator pos = m_SubsysOptionMap.find(sName);
    if (pos == m_SubsysOptionMap.end()) {
        throw Exception(AVG_ERR_OPTION_SUBSYS_UNKNOWN, sName);
    } else {
        return pos->second;
    }
}

void ConfigMgr::setOption(ConfigOptionVector& optionVector, 
        xmlDocPtr doc, xmlNodePtr pNode)
{
    string sName = (const char *)pNode->name;
    xmlChar * pVal = xmlNodeListGetString(doc, pNode->xmlChildrenNode, 1);
    string sValue = (const char *)pVal;
    xmlFree(pVal);
    setOption(optionVector, sName, sValue);    
}

void ConfigMgr::setOption(ConfigOptionVector& optionVector, 
        const string& sName, const string& sValue)
{
    for (unsigned int i = 0; i < optionVector.size(); i++) {
        if (optionVector[i].m_sName == sName) {
            optionVector[i].m_sValue = sValue;
            return;
        }
    }
    throw Exception(AVG_ERR_OPTION_UNKNOWN, sName);
}

void ConfigMgr::dump() const
{
    SubsysOptionMap::const_iterator it;
    for (it = m_SubsysOptionMap.begin(); it != m_SubsysOptionMap.end(); ++it) {
        cerr << (*it).first << ": " << endl;
        const ConfigOptionVector& SubsysOptions = (*it).second;
        for (unsigned int j = 0; j < SubsysOptions.size(); ++j) {
            cerr << "  " << SubsysOptions[j].m_sName << ": " 
                << SubsysOptions[j].m_sValue << endl;
        }
    }
}

string getGlobalConfigDir()
{
#ifdef _WIN32
    return getAvgLibPath()+"/etc/";
#else
    return "/etc/";
#endif
}

}

