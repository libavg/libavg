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

#ifndef _LinearAnim_H_
#define _LinearAnim_H_

#include "../api.h"

#include "SimpleAnim.h"

namespace avg {

class AVG_API LinearAnim: public SimpleAnim {
public:
    virtual ~LinearAnim();
    
    static AnimPtr create(const boost::python::object& node, const std::string& sAttrName,
            long long duration,
            const boost::python::object& pStartValue, 
            const boost::python::object& pEndValue, 
            bool bUseInt=false, 
            const boost::python::object& startCallback=boost::python::object(), 
            const boost::python::object& stopCallback=boost::python::object());

protected:
    LinearAnim(const boost::python::object& node, const std::string& sAttrName, 
            long long duration,
            const boost::python::object& pStartValue, 
            const boost::python::object& pEndValue, 
            bool bUseInt, 
            const boost::python::object& startCallback, 
            const boost::python::object& stopCallback);
    virtual double interpolate(double t);
    
private:
    double getStartPart(double start, double end, double cur);
};

AnimPtr fadeIn(const boost::python::object& node, long long duration, double max=1.0,
        const boost::python::object& stopCallback=boost::python::object());

AnimPtr fadeOut(const boost::python::object& node, long long duration, 
        const boost::python::object& stopCallback=boost::python::object());

}

#endif 



