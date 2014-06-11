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
//

#ifndef _ContinuousAnim_H_
#define _ContinuousAnim_H_

#include "../api.h"

#include "SimpleAnim.h"

namespace avg {

class AVG_API ContinuousAnim: public AttrAnim {
public:
    ContinuousAnim(const bp::object& node, const std::string& sAttrName, 
            const bp::object& startValue, 
            const bp::object& speed, 
            bool bUseInt=false, 
            const bp::object& startCallback=bp::object(),
            const bp::object& stopCallback=bp::object(),
            const bp::object& abortCallback=bp::object());
    virtual ~ContinuousAnim();
    
    virtual void start(bool bKeepAttr=false);
    virtual void abort();
    virtual bool step();

private:
    bp::object m_StartValue;
    bp::object m_Speed;
    bool m_bUseInt;
    
    bp::object m_EffStartValue;
    long long m_StartTime;
};

}

#endif 



