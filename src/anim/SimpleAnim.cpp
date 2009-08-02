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
      m_bRunning(false)
{
}

SimpleAnim::~SimpleAnim()
{
}

void SimpleAnim::setStartCallback(const object& startCallback)
{
    m_StartCallback = startCallback;
}

void SimpleAnim::setStopCallback(const object& stopCallback)
{
    m_StopCallback = stopCallback;
}

void SimpleAnim::start(bool bKeepAttr)
{
    abortAnim(m_Node, m_sAttrName);
    s_ActiveAnimations[ObjAttrID(m_Node, m_sAttrName)] = this;
    if (bKeepAttr) {
        m_StartTime = calcStartTime();
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
        onFrameEnd();
    }
}

void SimpleAnim::abort()
{
    if (m_bRunning) {
        remove();
    }
}

bool SimpleAnim::isRunning()
{
    return m_bRunning;
}

void SimpleAnim::onFrameEnd()
{
    double t = ((double(Player::get()->getFrameTime())-m_StartTime)
            /m_Duration);
    if (t > 1.0) {
        t = 1.0;
    }
    step(t);
}
    
double SimpleAnim::getStartTime() const
{
    return m_StartTime;
}

double SimpleAnim::getDuration() const
{
    return m_Duration;
}

object SimpleAnim::getValue() const
{
    return m_Node.attr(m_sAttrName.c_str());
}

void SimpleAnim::setValue(const object& val)
{
    m_Node.attr(m_sAttrName.c_str()) = val;
}

void SimpleAnim::step(double t) 
{
    if (t == 1.0) {
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
