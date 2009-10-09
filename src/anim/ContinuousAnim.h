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

#ifndef _ContinuousAnim_H_
#define _ContinuousAnim_H_

#include "../api.h"

#include "SimpleAnim.h"

namespace avg {

class AVG_API ContinuousAnim: public AttrAnim {
public:
    virtual ~ContinuousAnim();
    
    static AnimPtr create(const boost::python::object& node, const std::string& sAttrName,
            const boost::python::object& startValue, 
            const boost::python::object& speed, 
            bool bUseInt=false, 
            const boost::python::object& startCallback=boost::python::object(), 
            const boost::python::object& stopCallback=boost::python::object());
    
    virtual void start(bool bKeepAttr=false);
    virtual void abort();
    virtual bool step();

protected:
    ContinuousAnim(const boost::python::object& node, const std::string& sAttrName, 
            const boost::python::object& startValue, 
            const boost::python::object& speed, 
            bool bUseInt, 
            const boost::python::object& startCallback, 
            const boost::python::object& stopCallback);

private:
    boost::python::object m_StartValue;
    boost::python::object m_Speed;
    bool m_bUseInt;
    
    boost::python::object m_EffStartValue;
    long long m_StartTime;
};

}

#endif 



