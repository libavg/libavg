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

#ifndef _EaseInOutAnim_H_
#define _EaseInOutAnim_H_

#include "../api.h"

#include "SimpleAnim.h"

namespace avg {

class AVG_API EaseInOutAnim: public SimpleAnim {
public:
    virtual ~EaseInOutAnim();

    static AnimPtr create(const boost::python::object& node, const std::string& sAttrName, 
            long long duration,
            const boost::python::object& pStartValue, 
            const boost::python::object& pEndValue,
            long long easeInDuration, long long easeOutDuration, bool bUseInt=false, 
            const boost::python::object& startCallback=boost::python::object(), 
            const boost::python::object& stopCallback=boost::python::object());

protected:
    virtual double interpolate(double t);
    
private:
    EaseInOutAnim(const boost::python::object& node, const std::string& sAttrName, 
            long long duration,
            const boost::python::object& pStartValue, 
            const boost::python::object& pEndValue,
            long long easeInDuration, long long easeOutDuration, bool bUseInt, 
            const boost::python::object& startCallback, 
            const boost::python::object& stopCallback);
    double m_EaseInDuration;
    double m_EaseOutDuration;
};

}

#endif 



