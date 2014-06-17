//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2014 Ulrich von Zadow
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

#include "../base/Exception.h"
#include "../base/MathHelper.h"
#include "../player/Player.h"

using namespace boost;
using namespace boost::python;
using namespace std;

namespace avg {
   
SimpleAnim::SimpleAnim(const object& node, const string& sAttrName, long long duration, 
        const object& startValue, const object& endValue, bool bUseInt, 
        const object& startCallback, const object& stopCallback)
    : AttrAnim(node, sAttrName, startCallback, stopCallback),
      m_Duration(duration),
      m_StartValue(startValue),
      m_EndValue(endValue),
      m_bUseInt(bUseInt)
{
}

SimpleAnim::~SimpleAnim()
{
    if (Player::exists() && isRunning()) {
        setStopped();
    }
}

void SimpleAnim::start(bool bKeepAttr)
{
    AttrAnim::start();
    if (bKeepAttr) {
        m_StartTime = calcStartTime();
    } else {
        m_StartTime = Player::get()->getFrameTime();
    }
    if (m_Duration == 0) {
        setValue(m_EndValue);
        remove();
    } else {
        step();
    }
}

void SimpleAnim::abort()
{
    if (isRunning()) {
        remove();
    }
}

template<class T>
object typedLERP(const object& startValue, const object& endValue, float part)
{
    T start = extract<T>(startValue);
    T end = extract<T>(endValue);
    T cur = start+(end-start)*part;
    return object(cur);
}

bool SimpleAnim::step()
{
    assert(isRunning());
    float t = ((float(Player::get()->getFrameTime())-m_StartTime)
            /m_Duration);
    if (t >= 1.0) {
        setValue(m_EndValue);
        remove();
        return true;
    } else {
        object curValue;
        float part = interpolate(t);
        if (isPythonType<float>(m_StartValue)) {
            curValue = typedLERP<float>(m_StartValue, m_EndValue, part);
            if (m_bUseInt) {
                float d = extract<float>(curValue);
                curValue = object(round(d));
            }
        } else if (isPythonType<glm::vec2>(m_StartValue)) {
            curValue = typedLERP<glm::vec2>(m_StartValue, m_EndValue, part);
            if (m_bUseInt) {
                glm::vec2 pt = extract<glm::vec2>(curValue);
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

long long SimpleAnim::getStartTime() const
{
    return m_StartTime;
}

long long SimpleAnim::getDuration() const
{
    return m_Duration;
}

long long SimpleAnim::calcStartTime()
{
    float part;
    if (isPythonType<float>(m_StartValue)) {
        if (m_EndValue == m_StartValue) {
            part = 0;
        } else {
            part = getStartPart(extract<float>(m_StartValue), 
                    extract<float>(m_EndValue), extract<float>(getValue()));
        }
    } else if (isPythonType<glm::vec2>(m_StartValue)) {
        float start = glm::vec2(extract<glm::vec2>(m_StartValue)()).x;
        float end = glm::vec2(extract<glm::vec2>(m_EndValue)()).x;
        float cur = glm::vec2(extract<glm::vec2>(getValue())()).x;
        if (start == end) {
            start = glm::vec2(extract<glm::vec2>(m_StartValue)()).y;
            end = glm::vec2(extract<glm::vec2>(m_EndValue)()).y;
            cur = glm::vec2(extract<glm::vec2>(getValue())()).y;
        }
        if (start == end) {
            part = 0;
        } else {
            part = getStartPart(start, end, cur);
        }
    } else {
        throw (Exception(AVG_ERR_TYPE, 
                    "Animated attributes must be either numbers or Point2D."));
    }
    return Player::get()->getFrameTime()-(long long)(part*getDuration());
}

float SimpleAnim::getStartPart(float start, float end, float cur)
{
    float tstart = 0;
    float tend = 1;
    bool bDir = (start < end);
    for (int i=0; i<10; ++i) {
        float tmiddle = (tstart+tend)/2;
        float part = interpolate(tmiddle);
        float middle = start+(end-start)*part;
        if ((bDir && middle < cur) || (!bDir && middle >= cur)) {
            tstart = tmiddle;
        } else {
            tend = tmiddle;
        }
    }
    return (tend+tstart)/2;
}

void SimpleAnim::remove() 
{
    AnimPtr tempThis = shared_from_this();
    removeFromMap();
    setStopped();
}

}
