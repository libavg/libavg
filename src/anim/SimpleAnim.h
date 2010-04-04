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

#ifndef _SimpleAnim_H_
#define _SimpleAnim_H_

#include "../api.h"
// Python docs say python.h should be included before any standard headers (!)
#include "../player/WrapPython.h" 

#include "AttrAnim.h"
#include "../player/VisibleNode.h"

#include <boost/python.hpp>

#include <string>
#include <map>

namespace avg {

class SimpleAnim;
typedef boost::shared_ptr<class SimpleAnim> SimpleAnimPtr;

class AVG_API SimpleAnim: public AttrAnim 
{
public:
    SimpleAnim(const boost::python::object& node, const std::string& sAttrName,
            long long duration,
            const boost::python::object& pStartValue, 
            const boost::python::object& pEndValue, 
            bool bUseInt, 
            const boost::python::object& startCallback, 
            const boost::python::object& stopCallback);
    virtual ~SimpleAnim()=0;

    virtual void start(bool bKeepAttr=false);
    virtual void abort();
    virtual bool step();

protected:
    virtual double interpolate(double t)=0;
    void remove();
    
private:
    long long getStartTime() const;
    long long getDuration() const;
    long long calcStartTime();
    virtual double getStartPart(double start, double end, double cur);

    long long m_Duration;
    boost::python::object m_StartValue;
    boost::python::object m_EndValue;
    bool m_bUseInt;
    long long m_StartTime;
};

}

#endif 



