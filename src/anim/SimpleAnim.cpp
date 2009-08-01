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

#include "SimpleAnim.h"

#include "../player/Player.h"

using namespace boost::python;
using namespace std;

namespace avg {
   
SimpleAnim::AttrAnimationMap SimpleAnim::s_ActiveAnimations;

bool ObjAttrID::operator < (const ObjAttrID& other) const
{
    if (m_Node < other.m_Node) {
        return true;
    } else if (m_Node > other.m_Node) {
        return false;
    } else if (m_sAttrName < other.m_sAttrName) {
        return true;
    } else {
        return false;
    }
}

int SimpleAnim::getNumRunningAnims()
{
    return s_ActiveAnimations.size();
}

SimpleAnim::SimpleAnim(const object& node, const string& sAttrName, double duration, 
        bool bUseInt, const object& startCallback, const object& stopCallback)
    : m_Node(node),
      m_sAttrName(sAttrName),
      m_Duration(duration),
      m_bUseInt(bUseInt),
      m_StartCallback(startCallback),
      m_StopCallback(stopCallback),
      m_AbortCallback(0),
      m_bRunning(false)
{
}

SimpleAnim::~SimpleAnim()
{
}

void SimpleAnim::setHandler(const object& stopCallback, const object& abortCallback)
{
    m_StopCallback = stopCallback;
    m_AbortCallback = abortCallback;
}

void SimpleAnim::start(bool bKeepAttr)
{
    abortAnim(m_Node, m_sAttrName);
    s_ActiveAnimations[ObjAttrID(m_Node, m_sAttrName)] = this;
    if (bKeepAttr) {
        calcStartTime();
    } else {
        m_StartTime = Player::get()->getFrameTime();
    }
    Player::get()->registerFrameListener(this);
    m_bRunning = true;
    if (m_StartCallback != object()) {
        call<void>(m_StartCallback.ptr());
    }
    if (m_Duration == 0) {
        regularStop();
    } else {
        step();
    }
}

void SimpleAnim::abort()
{
    if (m_bRunning) {
        remove();
        if (m_AbortCallback != object()) {
            call<void>(m_AbortCallback.ptr());
        }
    }
}

bool SimpleAnim::isRunning()
{
    return m_bRunning;
}

void SimpleAnim::onFrameEnd()
{
    step();
}
    
double SimpleAnim::getStartTime()
{
    return m_StartTime;
}

double SimpleAnim::getDuration()
{
    return m_Duration;
}

void SimpleAnim::setValue(const object& val)
{
    m_Node.attr(m_sAttrName.c_str()) = val;
}

void SimpleAnim::step() 
{
    if (Player::get()->getFrameTime()-m_StartTime > m_Duration) {
        regularStop();
    }
}

void SimpleAnim::remove() 
{
    m_bRunning = false;
    s_ActiveAnimations.erase(ObjAttrID(m_Node, m_sAttrName));
    Player::get()->unregisterFrameListener(this);
    call<void>(m_StopCallback.ptr());
}

void SimpleAnim::abortAnim(const object& node, const string& sAttrName)
{
    ObjAttrID id(node, sAttrName);
    AttrAnimationMap::iterator it = s_ActiveAnimations.find(id);
    if (it != s_ActiveAnimations.end()) {
        it->second->remove();
    }
}


}
