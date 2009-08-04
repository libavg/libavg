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

#include "../player/Player.h"

using namespace boost::python;
using namespace std;

namespace avg {

AnimState::AnimState(const string& sName, AnimPtr pAnim, const string& sNextName,
            const object& enterCallback)
    : m_sName(sName),
      m_pAnim(pAnim),
      m_sNextName(sNextName)
{
}

AnimState::AnimState()
{
}

StateAnim::StateAnim(const vector<AnimState>& states)
    : Anim(object(), object()),
      m_bDebug(false)
{
    vector<AnimState>::const_iterator it;
    for (it=states.begin(); it != states.end(); ++it) {
        m_States[(*it).m_sName] = *it;
    }
}

StateAnim::~StateAnim()
{
    setState("");
}

void StateAnim::setState(const std::string& sName, bool bKeepAttr)
{
}

const std::string& StateAnim::getState() const
{
    return m_sCurStateName;
}

void StateAnim::setDebug(bool bDebug)
{
}
    

void StateAnim::onFrameEnd()
{
}

}
