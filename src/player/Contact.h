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

#ifndef _Contact_H_
#define _Contact_H_

#include "Publisher.h"

#include "../base/GLMHelper.h"

#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

// Python docs say python.h should be included before any standard headers (!)
#include "WrapPython.h"

#include <vector>
#include <map>

namespace avg {

class CursorEvent;
typedef boost::shared_ptr<class CursorEvent> CursorEventPtr;
class Contact;
typedef boost::shared_ptr<class Contact> ContactPtr;
class NodeChain;
typedef boost::shared_ptr<class NodeChain> NodeChainPtr;
class Node;
typedef boost::shared_ptr<class Node> NodePtr;

class AVG_API Contact: public Publisher {
public:
    static void registerType();
    Contact(CursorEventPtr pEvent);
    virtual ~Contact();

    int connectListener(PyObject* pMotionCallback, PyObject* pUpCallback);
    void disconnectListener(int id);

    long long getAge() const;
    float getDistanceFromStart() const;
    float getMotionAngle() const;
    glm::vec2 getMotionVec() const;
    float getDistanceTravelled() const;
    std::vector<CursorEventPtr> getEvents() const;

    void addEvent(CursorEventPtr pEvent);
    void sendEventToListeners(CursorEventPtr pCursorEvent);

    void setNodeChain(NodeChainPtr pNodeChain);
    glm::vec2 getRelPos(NodePtr pNode, const glm::vec2& pos) const;
    bool isNodeInTargets(NodePtr pNode) const;

    int getID() const;
    
private:
    void calcSpeed(CursorEventPtr pEvent, CursorEventPtr pOldEvent);
    void updateDistanceTravelled(CursorEventPtr pEvent1, CursorEventPtr pEvent2);
    void dumpListeners(std::string sFuncName);

    std::vector<CursorEventPtr> m_Events;

    bool m_bSendingEvents;

    struct Listener
    {
        Listener(PyObject * pMotionCallback, PyObject * pUpCallback);
        Listener(const Listener& other);
        ~Listener();
        PyObject* m_pMotionCallback;
        PyObject* m_pUpCallback;
    };

    static int s_LastListenerID;
    std::map<int, Listener> m_ListenerMap;
    int m_CurListenerID;
    bool m_bCurListenerIsDead;
    int m_CursorID;
    float m_DistanceTravelled;

    NodeChainPtr m_pNodeChain;
};

}

#endif
