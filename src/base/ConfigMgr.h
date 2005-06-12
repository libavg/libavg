//
// $Id$
// 

#ifndef _ConfigMgr_H_
#define _ConfigMgr_H_

#include "CmdLine.h"

#include <libxml/parser.h>

#include <string>
#include <vector>
#include <map>

namespace avg {

struct ConfigOption {
    ConfigOption(const std::string& sName, const std::string& sValue,
            const std::string& sDescription);

    std::string m_sName;
    std::string m_sValue;
    std::string m_sDescription;
};

typedef std::vector<ConfigOption> ConfigOptionVector;

class ConfigMgr {
public:
    static ConfigMgr* get();

    void printUsage();
    
    void addSubsys(const std::string& sName);
    void addOption(const std::string& sSubsys, const std::string& sName,
            const std::string& sDefault, const std::string& sDescription);
    void addGlobalOption(const std::string& sName, const std::string& sDefault, 
            const std::string& sDescription);
    
    void parseOptions(const CmdLine& cmdLine);

    const ConfigOptionVector* getOptions(const std::string& sSubsys) 
        const;
    const std::string* getOption(const std::string& sSubsys, 
            const std::string& sName) const;

    const ConfigOptionVector* getGlobalOptions() const;
    const std::string* getGlobalOption(const std::string& sName) const;

private:
    ConfigMgr();

    bool loadFile(const std::string& sPath);
    void loadFromCmdLine(const CmdLine& cmdLine);
    ConfigOptionVector& getSubsys(const std::string& sName);
    void setOption(ConfigOptionVector& OptionVector, 
            xmlDocPtr doc, xmlNodePtr pNode);
    void setOption(ConfigOptionVector& OptionVector, const std::string& sName,
            const std::string& sValue);
    void dump() const;

    typedef std::map<std::string, ConfigOptionVector> SubsysOptionMap;
    SubsysOptionMap m_SubsysOptionMap;

    ConfigOptionVector m_GlobalOptions;

    std::string m_sFName;

    static ConfigMgr* m_pGlobalConfigMgr;
};

}
#endif 

