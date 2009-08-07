//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2008 Ulrich von Zadow
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

#include "StateAnim.h"

#include "../base/Exception.h"
#include "../player/Player.h"

using namespace boost::python;
using namespace std;

namespace avg {

AnimState::AnimState(const string& sName, AnimPtr pAnim, const string& sNextName)
    : m_sName(sName),
      m_pAnim(pAnim),
      m_sNextName(sNextName)
{
}

AnimState::AnimState()
{
}

StateAnim::StateAnim(const vector<AnimState>& states)
    : GroupAnim(object(), object()),
      m_bDebug(false),
      m_bIsAborting(false)
{
    vector<AnimState>::const_iterator it;
    for (it=states.begin(); it != states.end(); ++it) {
        m_States[(*it).m_sName] = *it;
        it->m_pAnim->setParent(this);
    }
}

StateAnim::~StateAnim()
{
    setState("");
}
    
void StateAnim::abort()
{
    assert(false);
}

void StateAnim::setState(const std::string& sName, bool bKeepAttr)
{
    if (m_sCurStateName == sName) {
        return;
    }
    if (!m_sCurStateName.empty()) {
        m_bIsAborting = true;
        m_States[m_sCurStateName].m_pAnim->abort();
        m_bIsAborting = false;
    }
    switchToNewState(sName, bKeepAttr);
}

const std::string& StateAnim::getState() const
{
    return m_sCurStateName;
}

void StateAnim::setDebug(bool bDebug)
{
    m_bDebug = bDebug;
}
    

void StateAnim::childStopped(Anim* pChild)
{
    if(!m_bIsAborting) {
        const AnimState& curState = m_States[m_sCurStateName];
        switchToNewState(curState.m_sNextName, false);
    }
}

void StateAnim::switchToNewState(const string& sName, bool bKeepAttr)
{
    if (m_bDebug) {
        cerr << this << " State change: '" << m_sCurStateName << "' --> '" << sName 
                << "'" << endl;
    }
    if (!sName.empty()) {
        map<string, AnimState>::iterator it = m_States.find(sName);
        if (it == m_States.end()) {
            throw Exception(AVG_ERR_INVALID_ARGS, "StateAnim: State "+sName+" unknown.");
        } else {
            it->second.m_pAnim->start(bKeepAttr);
        }
    }
    m_sCurStateName = sName;
}

}
