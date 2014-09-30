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
#include "Player.h"

#include "BoostPython.h"
#include <boost/shared_ptr.hpp>

// Python docs say python.h should be included before any standard headers (!)
#include "WrapPython.h" 

#include <vector>

namespace avg {

class SubscriberInfo {
public:
    SubscriberInfo(int id, PyObject* pCallable);
    virtual ~SubscriberInfo();

    bool hasExpired() const;
    void invoke(py::list args) const;
    int getID() const;
    bool isCallable(const PyObject* pCallable) const;

private:
    int m_ID;
    PyObject* m_pWeakSelf;
    PyObject* m_pPyFunction;
    PyObject* m_pWeakClass;
};

typedef boost::shared_ptr<SubscriberInfo> SubscriberInfoPtr;

}
#endif


