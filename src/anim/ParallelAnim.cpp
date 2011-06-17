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

#include "ParallelAnim.h"

#include "../player/Player.h"

using namespace boost;
using namespace boost::python;
using namespace std;

namespace avg {

ParallelAnim::ParallelAnim(const vector<AnimPtr>& anims,
            const object& startCallback, const object& stopCallback, long long maxAge)
    : Anim(startCallback, stopCallback),
      m_Anims(anims),
      m_MaxAge(maxAge)
{
    vector<AnimPtr>::iterator it;
    for (it=m_Anims.begin(); it != m_Anims.end(); ++it) {
        (*it)->setHasParent();
    }
}

ParallelAnim::~ParallelAnim()
{
    if (Player::exists()) {
        abort();
    }
}

void ParallelAnim::start(bool bKeepAttr)
{
    Anim::start();
    m_StartTime = Player::get()->getFrameTime();

    vector<AnimPtr>::iterator it;
    for (it=m_Anims.begin(); it != m_Anims.end(); ++it) {
        (*it)->start(bKeepAttr);
        if ((*it)->isRunning()) {
            m_RunningAnims.push_back(*it);
        }
        m_This = dynamic_pointer_cast<ParallelAnim>(shared_from_this());
    }
}

void ParallelAnim::abort()
{
    if (isRunning()) {
        vector<AnimPtr>::iterator it;
        for (it=m_RunningAnims.begin(); it != m_RunningAnims.end(); ++it) {
            (*it)->abort();
        }
        m_RunningAnims.clear();
        setStopped();
        ParallelAnimPtr tempThis = m_This;
        m_This = ParallelAnimPtr();
        tempThis = ParallelAnimPtr();
    }
}
    
bool ParallelAnim::step()
{
    assert(isRunning());
    vector<AnimPtr>::iterator it;
    for (it=m_RunningAnims.begin(); it != m_RunningAnims.end(); ) {
        AnimPtr pAnim = (*it);
        bool bDone;
        if (pAnim->isRunning()) {
            bDone = pAnim->step();
        } else {
            bDone = true;
        }
        if (bDone) {
            it = m_RunningAnims.erase(it);
        } else {
            ++it;
        }
    }
    if (m_RunningAnims.empty()) {
        setStopped();
        ParallelAnimPtr tempThis = m_This;
        m_This = ParallelAnimPtr();
        tempThis = ParallelAnimPtr();
        return true;
    }
    if (m_MaxAge != -1 && Player::get()->getFrameTime()-m_StartTime >= m_MaxAge) {
        abort();
        return true;
    }
    return false;
}

}
