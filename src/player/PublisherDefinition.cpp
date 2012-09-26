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

#include "PublisherDefinition.h"

#include "PublisherDefinitionRegistry.h"

#include "../base/Exception.h"

using namespace std;

namespace avg {

PublisherDefinition::PublisherDefinition(const string& sName, const string& sBaseName)
    : m_sName(sName)
{
    if (sBaseName != "") {
        PublisherDefinitionPtr pBaseDef = 
                PublisherDefinitionRegistry::get()->getDefinition(sBaseName);
        m_MessageIDs = pBaseDef->m_MessageIDs;
    }
}

PublisherDefinition::~PublisherDefinition()
{
}

PublisherDefinitionPtr PublisherDefinition::create(const std::string& sName, 
            const std::string& sBaseName)
{
    PublisherDefinitionPtr pDef(new PublisherDefinition(sName, sBaseName));
    PublisherDefinitionRegistry::get()->registerDefinition(pDef);
    return pDef;
}

void PublisherDefinition::addMessage(const std::string& sName)
{
    m_MessageIDs.push_back(PublisherDefinitionRegistry::get()->genMessageID(sName));
}

const MessageID& PublisherDefinition::getMessageID(const std::string& sName) const
{
    for (unsigned i=0; i<m_MessageIDs.size(); ++i) {
        if (m_MessageIDs[i].m_sName == sName) {
            return m_MessageIDs[i];
        }
    }
    AVG_ASSERT_MSG(false, (string("Message named '")+sName+("' unknown.")).c_str());
    // Avoid compiler warning.
    static MessageID nullMsg("", -1);
    return nullMsg;
}

const std::vector<MessageID>& PublisherDefinition::getMessageIDs() const
{
    return m_MessageIDs;
}

const std::string& PublisherDefinition::getName() const
{
    return m_sName;
}
    
void PublisherDefinition::dump() const
{
    cerr << m_sName << endl;
    for (unsigned i=0; i<m_MessageIDs.size(); ++i) {
        cerr << "  " << m_MessageIDs[i].m_sName << ": " << m_MessageIDs[i].m_ID << endl;
    }
}

}
