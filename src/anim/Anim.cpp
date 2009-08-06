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

#include "Anim.h"

#include "GroupAnim.h"

#include "../base/Exception.h"
#include "../player/Player.h"

using namespace boost::python;
using namespace std;

namespace avg {
   
Anim::Anim(const object& startCallback, const object& stopCallback)
    : m_StartCallback(startCallback),
      m_StopCallback(stopCallback),
      m_bRunning(false),
      m_pParent(0)
{
}

Anim::~Anim()
{
}

void Anim::setStartCallback(const object& startCallback)
{
    m_StartCallback = startCallback;
}

void Anim::setStopCallback(const object& stopCallback)
{
    m_StopCallback = stopCallback;
}

void Anim::start(bool)
{
    if (!(Player::get()->isPlaying())) {
        throw(Exception(AVG_ERR_UNSUPPORTED, 
                "Animation playback can only be started when the player is running."));
    }
    m_bRunning = true;
    if (m_StartCallback != object()) {
        call<void>(m_StartCallback.ptr());
    }
}

bool Anim::isRunning() const
{
    return m_bRunning;
}

void Anim::setParent(GroupAnim* pParent)
{
    m_pParent = pParent;
}

void Anim::setStopped()
{
    m_bRunning = false;
    if (m_pParent) {
        m_pParent->childStopped(this);
    }
    if (m_StopCallback != object()) {
        call<void>(m_StopCallback.ptr());
    }
}

}
