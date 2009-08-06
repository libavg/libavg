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

#ifndef _Anim_H_
#define _Anim_H_

#include "../api.h"
// Python docs say python.h should be included before any standard headers (!)
#include "../player/WrapPython.h" 

#include "../player/Node.h"

#include <boost/python.hpp>
#include <boost/shared_ptr.hpp>

#include <string>
#include <map>

namespace avg {

class GroupAnim;

class AVG_API Anim {
public:
    static int getNumRunningAnims();

    Anim(const boost::python::object& startCallback, 
            const boost::python::object& stopCallback);
    virtual ~Anim()=0;

    void setStartCallback(const boost::python::object& startCallback);
    void setStopCallback(const boost::python::object& stopCallback);
    virtual void start(bool bKeepAttr=false);
    virtual void abort() = 0;
    bool isRunning() const;
    void setParent(GroupAnim* pParent);

protected:
    void setStopped();
    
private:
    boost::python::object m_StartCallback;
    boost::python::object m_StopCallback;
    bool m_bRunning;
    GroupAnim* m_pParent;
};

typedef boost::shared_ptr<class Anim> AnimPtr;

}

#endif 



