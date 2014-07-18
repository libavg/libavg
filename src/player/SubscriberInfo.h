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

#ifndef _SubscriberInfo_H_
#define _SubscriberInfo_H_

#include "../api.h"
#include "../base/IPlaybackEndListener.h"
#include "Player.h"

#include "BoostPython.h"
#include <boost/shared_ptr.hpp>

// Python docs say python.h should be included before any standard headers (!)
#include "WrapPython.h" 

#include <vector>


namespace avg {

class PyMethodRef;

class SubscriberInfo {
public:
    SubscriberInfo(int id, const py::object& callable);
    virtual ~SubscriberInfo();

    bool hasExpired() const;
    void invoke(py::list args) const;
    int getID() const;
    bool isCallable(const py::object& callable) const;

private:
    friend class PyMethodRef;
    int m_ID;
    py::object m_Callable;
    static boost::shared_ptr<PyMethodRef> s_pPyMethodref;
};

typedef boost::shared_ptr<SubscriberInfo> SubscriberInfoPtr;


class PyMethodRef : public IPlaybackEndListener  {
public:
    PyMethodRef() {
        m_MethodrefModule = py::import("libavg.methodref");
        Player::get()->registerPlaybackEndListener(this);
    }

    virtual void onPlaybackEnd(){
        SubscriberInfo::s_pPyMethodref.reset();
    }

    py::object getMethodRef(const py::object& callable){
        return py::object(m_MethodrefModule.attr("methodref")(callable));
    }

private:
    py::object m_MethodrefModule;
};

}
#endif


