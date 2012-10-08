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

#include "PublisherDefinitionRegistry.h"

#include "PublisherDefinition.h"

#include "../base/Exception.h"

using namespace std;


namespace avg {

PublisherDefinitionRegistry* PublisherDefinitionRegistry::s_pInstance = 0;

PublisherDefinitionRegistry::PublisherDefinitionRegistry()
    : m_LastMessageID(-1)
{
    s_pInstance = this;
    // The following should really happen in the player constructor, but the entries
    // need to exist for the player to be constructed.
    PublisherDefinitionPtr pPlayerDef = PublisherDefinition::create("Player");
    pPlayerDef->addMessage("KEY_DOWN");
    pPlayerDef->addMessage("KEY_UP");
    pPlayerDef->addMessage("PLAYBACK_START");
    pPlayerDef->addMessage("PLAYBACK_END");
    pPlayerDef->addMessage("ON_FRAME");
}

PublisherDefinitionRegistry::~PublisherDefinitionRegistry()
{
}

PublisherDefinitionRegistry* PublisherDefinitionRegistry::get()
{
    if (!s_pInstance) {
        new PublisherDefinitionRegistry();
    }
    return s_pInstance;
}

void PublisherDefinitionRegistry::registerDefinition(PublisherDefinitionPtr def)
{
    m_Definitions.push_back(def);
}

PublisherDefinitionPtr PublisherDefinitionRegistry::getDefinition(const string& sName) 
        const
{
    for (unsigned i=0; i<m_Definitions.size(); ++i) {
        if (m_Definitions[i]->getName() == sName) {
            return m_Definitions[i];
        }
    }
    AVG_ASSERT_MSG(false, (string("Can't find PublisherDefinition ")+sName).c_str());
    return PublisherDefinitionPtr();
}
    
void PublisherDefinitionRegistry::dump() const
{
    for (unsigned i=0; i<m_Definitions.size(); ++i) {
        m_Definitions[i]->dump();
    }
}

MessageID PublisherDefinitionRegistry::genMessageID(const string& sName)
{
    return MessageID(sName, ++m_LastMessageID);
}

}
