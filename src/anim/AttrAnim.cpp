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

#include "AttrAnim.h"

#include "../base/Exception.h"
#include "../player/Player.h"
#include "../player/Node.h"

using namespace boost;
using namespace boost::python;
using namespace std;

namespace avg {

AttrAnim::AttrAnimationMap AttrAnim::s_ActiveAnimations;

bool ObjAttrID::operator < (const ObjAttrID& other) const
{
    if (m_ObjHash < other.m_ObjHash) {
        return true;
    } else if (m_ObjHash > other.m_ObjHash) {
        return false;
    } else if (m_sAttrName < other.m_sAttrName) {
        return true;
    } else {
        return false;
    }
}

int AttrAnim::getNumRunningAnims()
{
    return s_ActiveAnimations.size();
}

AttrAnim::AttrAnim(const object& node, const string& sAttrName, 
        const object& startCallback, const object& stopCallback)
    : Anim(startCallback, stopCallback),
      m_Node(node),
      m_sAttrName(sAttrName)
{
    object obj = getValue();
}

AttrAnim::~AttrAnim()
{
}

void AttrAnim::start(bool bKeepAttr)
{
    stopActiveAttrAnim();
    Anim::start();
    addToMap();
}

object AttrAnim::getValue() const
{
    return m_Node.attr(m_sAttrName.c_str());
}

void AttrAnim::setValue(const object& val)
{
    m_Node.attr(m_sAttrName.c_str()) = val;
}

void AttrAnim::addToMap()
{
    s_ActiveAnimations[ObjAttrID(m_Node, m_sAttrName)] = 
            dynamic_pointer_cast<AttrAnim>(shared_from_this());
}

void AttrAnim::removeFromMap()
{
    s_ActiveAnimations.erase(ObjAttrID(m_Node, m_sAttrName));
}

void AttrAnim::stopActiveAttrAnim()
{
    ObjAttrID id(m_Node, m_sAttrName);
    AttrAnimationMap::iterator it = s_ActiveAnimations.find(id);
    if (it != s_ActiveAnimations.end()) {
        it->second->abort();
    }
}

}
