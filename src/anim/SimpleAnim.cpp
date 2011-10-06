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
object typedLERP(const object& startValue, const object& endValue, double part)
{
    T start = extract<T>(startValue);
    T end = extract<T>(endValue);
    T cur = start+(end-start)*part;
    return object(cur);
}

bool SimpleAnim::step()
{
    assert(isRunning());
    double t = ((double(Player::get()->getFrameTime())-m_StartTime)
            /m_Duration);
    if (t >= 1.0) {
        setValue(m_EndValue);
        remove();
        return true;
    } else {
        object curValue;
        double part = interpolate(t);
        if (isPythonType<double>(m_StartValue)) {
            curValue = typedLERP<double>(m_StartValue, m_EndValue, part);
            if (m_bUseInt) {
                double d = extract<double>(curValue);
                curValue = object(round(d));
            }
        } else if (isPythonType<DPoint>(m_StartValue)) {
            curValue = typedLERP<DPoint>(m_StartValue, m_EndValue, part);
            if (m_bUseInt) {
                DPoint pt = extract<DPoint>(curValue);
                curValue = object(DPoint(round(pt.x), round(pt.y)));
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
    double part;
    if (isPythonType<double>(m_StartValue)) {
        if (m_EndValue == m_StartValue) {
            part = 0;
        } else {
            part = getStartPart(extract<double>(m_StartValue), 
                    extract<double>(m_EndValue), extract<double>(getValue()));
        }
    } else if (isPythonType<DPoint>(m_StartValue)) {
        double start = DPoint(extract<DPoint>(m_StartValue)).x;
        double end = DPoint(extract<DPoint>(m_EndValue)).x;
        double cur = DPoint(extract<DPoint>(getValue())).x;
        if (start == end) {
            start = DPoint(extract<DPoint>(m_StartValue)).y;
            end = DPoint(extract<DPoint>(m_EndValue)).y;
            start = DPoint(extract<DPoint>(getValue())).y;
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

double SimpleAnim::getStartPart(double start, double end, double cur)
{
    double tstart = 0;
    double tend = 1;
    bool bDir = (start < end);
    for (int i=0; i<10; ++i) {
        double tmiddle = (tstart+tend)/2;
        double part = interpolate(tmiddle);
        double middle = start+(end-start)*part;
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
