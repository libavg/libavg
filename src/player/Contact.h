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

#ifndef _Contact_H_
#define _Contact_H_

#include "../base/Point.h"

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

// Python docs say python.h should be included before any standard headers (!)
#include "WrapPython.h"

#include <vector>

namespace avg {

class CursorEvent;
typedef boost::shared_ptr<class CursorEvent> CursorEventPtr;
class Contact;
typedef boost::shared_ptr<class Contact> ContactPtr;
typedef boost::weak_ptr<class Contact> ContactWeakPtr;

class Contact {
public:
    Contact(CursorEventPtr pEvent, bool bProcessEvents = true);
    virtual ~Contact();
    void disconnectEverything();

    void setThis(ContactWeakPtr This);
    ContactPtr getThis() const;

    void connectListener(PyObject* pListener);
    void disconnectListener(PyObject* pListener);

    long long getAge() const;
    double getDistanceFromStart() const;
    double getMotionAngle() const;
    DPoint getMotionVec() const;
    double getDistanceTravelled() const;

    void pushEvent(CursorEventPtr pEvent, bool bCheckMotion=true);
    void addEvent(CursorEventPtr pEvent);
    CursorEventPtr pollEvent();
    CursorEventPtr getLastEvent();
    bool hasListeners() const;
    void sendEventToListeners(CursorEventPtr pEvent);

    int getID() const;

private:
    void calcSpeed(CursorEventPtr pEvent, CursorEventPtr pOldEvent);
    void updateDistanceTravelled(CursorEventPtr pEvent1, CursorEventPtr pEvent2);

    bool m_bProcessEvents;  // False for mouse contacts, where event polling is handled 
                            // externally
    CursorEventPtr m_pFirstEvent;
    CursorEventPtr m_pLastEvent;
    std::vector<CursorEventPtr> m_pNewEvents;

    bool m_bFirstFrame;
    ContactWeakPtr m_This;
    bool m_bSendingEvents;

    std::vector<PyObject*> m_pListeners;
    std::vector<PyObject*> m_pDeadListeners;
    int m_CursorID;
    double m_DistanceTravelled;
};

}

#endif
