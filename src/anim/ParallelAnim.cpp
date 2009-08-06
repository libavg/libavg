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

using namespace boost::python;
using namespace std;

namespace avg {

ParallelAnim::ParallelAnim(const vector<AnimPtr>& anims,
            const object& startCallback, const object& stopCallback, long long maxAge)
    : GroupAnim(startCallback, stopCallback),
      m_Anims(anims),
      m_MaxAge(maxAge)
{
    vector<AnimPtr>::iterator it;
    for (it=m_Anims.begin(); it != m_Anims.end(); ++it) {
        (*it)->setParent(this);
    }
}

ParallelAnim::~ParallelAnim()
{
}

void ParallelAnim::start(bool bKeepAttr)
{
    Anim::start();
    m_StartTime = Player::get()->getFrameTime();
    Player::get()->registerPreRenderListener(this);

    vector<AnimPtr>::iterator it;
    for (it=m_Anims.begin(); it != m_Anims.end(); ++it) {
        (*it)->start(bKeepAttr);
    }
}

void ParallelAnim::abort()
{
    if (isRunning()) {
//        Player::get()->unregisterPreRenderListener(this);
        vector<AnimPtr>::iterator it;
        for (it=m_Anims.begin(); it != m_Anims.end(); ++it) {
            (*it)->abort();
        }
//        setStopped();
    }
}
    
void ParallelAnim::onPreRender()
{
    assert(isRunning());
    if (m_MaxAge != -1 && Player::get()->getFrameTime()-m_StartTime >= m_MaxAge) {
        abort();
    }
}

void ParallelAnim::childStopped(Anim* pChild)
{
    bool bFound = false;
    vector<AnimPtr>::iterator it;
    for (it=m_Anims.begin(); it != m_Anims.end() && !bFound; ++it) {
        if (&(**it) == pChild) {
            bFound = true;
        }
    }
    assert(bFound);

    bool bAllDone = true;
    for (it=m_Anims.begin(); it != m_Anims.end() && bAllDone; ++it) {
        if ((*it)->isRunning()) {
            bAllDone = false;
        }
    }
    if (bAllDone) {
        Player::get()->unregisterPreRenderListener(this);
        setStopped();
    }
}

}
