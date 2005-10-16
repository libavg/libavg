//
// $Id$
//

#include "ConfigMgr.h"
#include "Logger.h"
#include "Exception.h"

#include <libxml/xmlmemory.h>

#include <iostream>
#include <stdlib.h>
#include <unistd.h>

// TODO: Refactor - this stuff is left over from pre-python days.

using namespace std;
 
namespace avg {
ConfigOption::ConfigOption(const string& sName, const string& sValue,
            const string& sDescription)
    : m_sName(sName),
      m_sValue(sValue),
      m_sDescription(sDescription)
{
}

ConfigMgr* ConfigMgr::m_pGlobalConfigMgr = 0;

ConfigMgr* ConfigMgr::get()
{
    if (!m_pGlobalConfigMgr) {
        m_pGlobalConfigMgr = new ConfigMgr;
    }
    return m_pGlobalConfigMgr;
}

ConfigMgr::ConfigMgr()
{
    addSubsys("scr");
    addOption("scr", "subsys", "OGL",
            "Whether to use OpenGL (OGL) or DirectFB (DFB) for video output");
    addOption("scr", "bpp", "24",
            "Screen bits per pixel. Valid values are 15, 16, 24 and 32.");
    addOption("scr", "fullscreen", "false",
            "Whether to run fullscreen (true) or in a window (false).");
    addOption("scr", "windowwidth", "0",
            "The width of the window to use. Contents are scaled.");
    addOption("scr", "windowheight", "0",
            "The height of the window to use. Contents are scaled.");

    m_sFName = "avgrc";
    bool bOk1 = loadFile("/etc/"+m_sFName);
    char * pHome = getenv("HOME");
    bool bOk2;
    if (!pHome) {
        AVG_TRACE(Logger::WARNING, "No home directory set.");
        bOk2 = false;
    } else {
        bOk2 = loadFile(string(pHome)+"/."+m_sFName);
    }
    if (!bOk1 && !bOk2) {
        AVG_TRACE(Logger::ERROR,
                "Neither /etc/avgrc nor ~/.avgrc was found. If");
        AVG_TRACE(Logger::ERROR,
                "this is your initial install, you need to");
        AVG_TRACE(Logger::ERROR,
                "copy src/avgrc from the package directory");
        AVG_TRACE(Logger::ERROR,
                "to /etc. Have a look at the contents to");
        AVG_TRACE(Logger::ERROR,
                "check if everything is set correctly.");
        AVG_TRACE(Logger::ERROR,
                "Aborting.");
        exit(255);
    }
}

void ConfigMgr::addSubsys(const string& sName)
{
    m_SubsysOptionMap[sName] = ConfigOptionVector();
}

void ConfigMgr::addOption(const string& sSubsys, const string& sName,
        const std::string& sDefault, const string& sDescription)
{
    ConfigOptionVector& Subsys = m_SubsysOptionMap[sSubsys];
    Subsys.push_back(ConfigOption(sName, sDefault, sDescription));
}

void ConfigMgr::addGlobalOption(const string& sName, const string& sDefault,
        const string& sDescription)
{
    m_GlobalOptions.push_back(ConfigOption(sName, sDefault, sDescription));
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

const ConfigOptionVector* ConfigMgr::getGlobalOptions() const
{
    return &m_GlobalOptions;
}

const string* ConfigMgr::getGlobalOption(const string& sName) const
{
    for (unsigned int i=0; i<m_GlobalOptions.size(); i++) {
        if (m_GlobalOptions[i].m_sName == sName) {
            return &m_GlobalOptions[i].m_sValue;
        }
    }
    return 0;

}

bool ConfigMgr::loadFile(const std::string& sPath) {
    string sSubsys;
    try {
        int err = access(sPath.c_str(), R_OK);
        if (err == -1) {
            if (errno == EACCES) {
                AVG_TRACE(Logger::WARNING,
                        sPath+": File exists, but process doesn't have read permissions!");
            }
            return false;
        }
        xmlDocPtr doc;
        doc = xmlParseFile(sPath.c_str());
        if (!doc) {
            return false;
        }
        xmlNodePtr pRoot = xmlDocGetRootElement(doc);
        if (xmlStrcmp(pRoot->name, (const xmlChar *)(m_sFName.c_str()))) {
            AVG_TRACE(Logger::ERROR, 
                    sPath+": Root node must be <"+m_sFName+">, found " 
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
                    AVG_TRACE(Logger::ERROR,
                            sPath << ": Option " << sSubsys
                            << " has no value. Ignoring.");
                } else {
                    if (!xmlStrcmp(pOptionNode->name, (const xmlChar *)"text") &&
                            !pOptionNode->next)
                    {   // This is a global option, not a list of subsystem options.
                        setOption(m_GlobalOptions, doc, pSubsysNode);
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
            }
            pSubsysNode = pSubsysNode->next;
        }
    } catch (Exception& e) {
        switch (e.GetCode()) {
            case AVG_ERR_OPTION_SUBSYS_UNKNOWN:
                AVG_TRACE(Logger::ERROR, "While parsing " << sPath 
                        << ": Option group " << e.GetStr() << " unknown. Aborting.");
                exit(255);
            case AVG_ERR_OPTION_UNKNOWN: 
                AVG_TRACE(Logger::ERROR, "While parsing " << sPath 
                        << ": Option " << sSubsys << ":" << e.GetStr() 
                        << " unknown. Aborting.");
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

void ConfigMgr::setOption(ConfigOptionVector& OptionVector, 
        xmlDocPtr doc, xmlNodePtr pNode)
{
    string sName = (const char *)pNode->name;
    string sValue = 
            (const char *)xmlNodeListGetString(doc, pNode->xmlChildrenNode, 1);
    setOption(OptionVector, sName, sValue);    
}

void ConfigMgr::setOption(ConfigOptionVector& OptionVector, 
        const string& sName, const string& sValue)
{
    // TODO: Change OptionVector into a map?
    for (unsigned int i=0; i<OptionVector.size(); i++) {
        if (OptionVector[i].m_sName == sName) {
            OptionVector[i].m_sValue = sValue;
            return;
        }
    }
    throw Exception(AVG_ERR_OPTION_UNKNOWN, sName);
}

void ConfigMgr::dump() const
{
    cerr << "Global options: " << endl;
    for (unsigned int i=0; i<m_GlobalOptions.size(); ++i) {
        cerr << "  " << m_GlobalOptions[i].m_sName << ": " 
                << m_GlobalOptions[i].m_sValue << endl;
    }
    SubsysOptionMap::const_iterator it;
    for (it=m_SubsysOptionMap.begin(); it != m_SubsysOptionMap.end(); ++it) {
        cerr << (*it).first << ": " << endl;
        const ConfigOptionVector& SubsysOptions = (*it).second;
        for (unsigned int j=0; j<SubsysOptions.size(); ++j) {
            cerr << "  " << SubsysOptions[j].m_sName << ": " 
                << SubsysOptions[j].m_sValue << endl;
        }
    }
}

}

