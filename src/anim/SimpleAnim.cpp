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

#include "../base/Exception.h"
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

SimpleAnim::SimpleAnim(const object& node, const string& sAttrName, long long duration, 
        const object& startValue, const object& endValue, bool bUseInt, 
        const object& startCallback, const object& stopCallback)
    : Anim(startCallback, stopCallback),
      m_Node(node),
      m_sAttrName(sAttrName),
      m_Duration(duration),
      m_StartValue(startValue),
      m_EndValue(endValue),
      m_bUseInt(bUseInt)
{
    object obj = getValue();
}

SimpleAnim::~SimpleAnim()
{
}

void SimpleAnim::start(bool bKeepAttr)
{
    Anim::start();
    abortAnim(m_Node, m_sAttrName);
    s_ActiveAnimations[ObjAttrID(m_Node, m_sAttrName)] = this;
    if (bKeepAttr) {
        m_StartTime = calcStartTime();
    } else {
        m_StartTime = Player::get()->getFrameTime();
    }
    Player::get()->registerPreRenderListener(this);
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

void SimpleAnim::onPreRender()
{
    step();
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

template<class T>
bool isPythonType(const object& obj)
{
    extract<T> ext(obj);
    return ext.check();
}

template<class T>
object typedLERP(const object& startValue, const object& endValue, double part)
{
    T start = extract<T>(startValue);
    T end = extract<T>(endValue);
    T cur = start+(end-start)*part;
    return object(cur);
}

void SimpleAnim::step()
{
    double t = ((double(Player::get()->getFrameTime())-m_StartTime)
            /m_Duration);
    if (t >= 1.0) {
        setValue(m_EndValue);
        remove();
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
    }
}

long long SimpleAnim::calcStartTime()
{
    double part;
    if (isPythonType<double>(m_StartValue)) {
        part = extract<double>((getValue()-m_StartValue)/(m_EndValue-m_StartValue));
    } else if (isPythonType<DPoint>(m_StartValue)) {
        double start = DPoint(extract<DPoint>(m_StartValue)).x;
        double end = DPoint(extract<DPoint>(m_EndValue)).x;
        double cur = DPoint(extract<DPoint>(getValue())).x;
        part = (cur-start)/(end-start);
    } else {
        throw (Exception(AVG_ERR_TYPE, 
                    "Animated attributes must be either numbers or Point2D."));
    }
    return (long long)(Player::get()->getFrameTime()-part*getDuration());
}

void SimpleAnim::remove() 
{
    s_ActiveAnimations.erase(ObjAttrID(m_Node, m_sAttrName));
    Player::get()->unregisterPreRenderListener(this);
    setStopped();
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
