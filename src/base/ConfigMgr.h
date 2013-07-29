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

#ifndef _ConfigMgr_H_
#define _ConfigMgr_H_

#include "../api.h"

#include "GLMHelper.h"
#include <libxml/parser.h>

#include <string>
#include <vector>
#include <map>

namespace avg {

struct ConfigOption {
    ConfigOption(const std::string& sName, const std::string& sValue);

    std::string m_sName;
    std::string m_sValue;
};

typedef std::vector<ConfigOption> ConfigOptionVector;

class AVG_API ConfigMgr {
public:
    static ConfigMgr* get();

    void addSubsys(const std::string& sName);
    void addOption(const std::string& sSubsys, const std::string& sName,
            const std::string& sDefault);

    const ConfigOptionVector* getOptions(const std::string& sSubsys) const;
    const std::string* getOption(const std::string& sSubsys, 
            const std::string& sName) const;
    bool getBoolOption(const std::string& sSubsys, 
            const std::string& sName, bool bDefault) const;
    int getIntOption(const std::string& sSubsys, 
            const std::string& sName, int Default) const;
    void getGammaOption(const std::string& sSubsys, 
            const std::string& sName, float* Val) const;
    glm::vec2 getSizeOption(const std::string& sSubsys, 
            const std::string& sName) const;
    void getStringOption(const std::string& sSubsys, 
            const std::string& sName, const std::string& sDefault, std::string& sVal) 
            const;

    void dump() const;

private:
    ConfigMgr();

    bool loadFile(const std::string& sPath);
    ConfigOptionVector& getSubsys(const std::string& sName);
    void setOption(ConfigOptionVector& optionVector, 
            xmlDocPtr doc, xmlNodePtr pNode);
    void setOption(ConfigOptionVector& optionVector, const std::string& sName,
            const std::string& sValue);

    typedef std::map<std::string, ConfigOptionVector> SubsysOptionMap;
    SubsysOptionMap m_SubsysOptionMap;

    std::string m_sFName;

    static ConfigMgr* m_pGlobalConfigMgr;
    friend void deleteConfigMgr();
};

std::string getGlobalConfigDir();

}
#endif 

