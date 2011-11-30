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

#include "ContinuousAnim.h"

#include "../base/Exception.h"
#include "../base/MathHelper.h"
#include "../player/Player.h"

using namespace boost::python;
using namespace std;

namespace avg {

ContinuousAnim::ContinuousAnim(const object& node, const string& sAttrName, 
            const object& startValue, const object& speed, bool bUseInt, 
            const object& startCallback, const object& stopCallback)
    : AttrAnim(node, sAttrName, startCallback, stopCallback),
      m_StartValue(startValue),
      m_Speed(speed),
      m_bUseInt(bUseInt)
{
}

ContinuousAnim::~ContinuousAnim()
{
}

void ContinuousAnim::start(bool bKeepAttr)
{
    AttrAnim::start();
    if (!bKeepAttr) {
        setValue(m_StartValue);
    }
    m_EffStartValue = getValue();
    m_StartTime = Player::get()->getFrameTime();
}

void ContinuousAnim::abort()
{
    if (isRunning()) {
        AnimPtr tempThis = shared_from_this();
        removeFromMap();
        setStopped();
    }
}

bool ContinuousAnim::step()
{
    object curValue;
    float time = (Player::get()->getFrameTime()-m_StartTime)/1000.0f;
    if (isPythonType<float>(m_EffStartValue)) {
        curValue = object(time*extract<float>(m_Speed)+m_EffStartValue);
        if (m_bUseInt) {
            float d = extract<float>(curValue);
            curValue = object(round(d));
        }
    } else if (isPythonType<glm::vec2>(m_EffStartValue)) {
        glm::vec2 pt = extract<glm::vec2>(m_Speed)();
        curValue = object(time*pt+m_EffStartValue);
        if (m_bUseInt) {
            glm::vec2 pt = extract<glm::vec2>(curValue)();
            curValue = object(glm::vec2(round(pt.x), round(pt.y)));
        }
    } else {
        throw (Exception(AVG_ERR_TYPE, 
                    "Animated attributes must be either numbers or Point2D."));
    }
    setValue(curValue);
    return false;
}

}
