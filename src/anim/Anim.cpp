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

#include "../base/Exception.h"
#include "../base/ObjectCounter.h"

#include "../player/Player.h"

using namespace boost::python;
using namespace std;

namespace avg {
   
Anim::Anim(const object& startCallback, const object& stopCallback)
    : m_StartCallback(startCallback),
      m_StopCallback(stopCallback),
      m_bRunning(false),
      m_bIsRoot(true)
{
    ObjectCounter::get()->incRef(&typeid(*this));
    Player::get()->registerPlaybackEndListener(this);
}

Anim::~Anim()
{
    ObjectCounter::get()->decRef(&typeid(*this));
    if (Player::exists()) {
        Player::get()->unregisterPlaybackEndListener(this);
    }
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
    if (m_bRunning) {
        throw(Exception(AVG_ERR_UNSUPPORTED, 
                "Anim.start(): animation already running."));
    }
    if (!(Player::get()->isPlaying())) {
        throw(Exception(AVG_ERR_UNSUPPORTED, 
                "Animation playback can only be started when the player is running."));
    }
    m_bRunning = true;
    if (m_bIsRoot) {
        Player::get()->registerPreRenderListener(this);
    }
    if (m_StartCallback != object()) {
        call<void>(m_StartCallback.ptr());
    }
}

bool Anim::isRunning() const
{
    return m_bRunning;
}

void Anim::setHasParent()
{
    assert(!m_bRunning);
    m_bIsRoot = false;
}

void Anim::onPreRender()
{
    step();
}
    
void Anim::onPlaybackEnd()
{
    AnimPtr tempThis = shared_from_this();
    if (m_bRunning) {
        abort();
    }
    m_StartCallback = object();
    m_StopCallback = object();
}

void Anim::setStopped()
{
    if (m_bIsRoot) {
        Player::get()->unregisterPreRenderListener(this);
    }
    m_bRunning = false;
    if (m_StopCallback != object()) {
        try {
            m_StopCallback();
        } catch (error_already_set &) {
            cerr << "Python exception in Anim stop callback." << endl;
            PyErr_Print();
            exit(5);
        }
    }
}

}
