//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2011 Ulrich von Zadow
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
//  Original author of this file is igor@c-base.org
//

#ifndef _CursorEvent_h_
#define _CursorEvent_h_

#include "../api.h"
#include "Event.h"

#include "../base/GLMHelper.h"

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

namespace avg {

const int MOUSECURSORID=-1;

class AVG_API CursorEvent;
typedef boost::shared_ptr<class CursorEvent> CursorEventPtr;

class AVG_API Contact;
typedef boost::shared_ptr<class Contact> ContactPtr;
typedef boost::weak_ptr<class Contact> ContactWeakPtr;

class Node;
typedef boost::shared_ptr<class Node> NodePtr;
typedef boost::weak_ptr<class Node> NodeWeakPtr;

class AVG_API CursorEvent: public Event 
{
    public:
        CursorEvent(int id, Type eventType, const IntPoint& position, Source source,
                int when=-1);
        virtual ~CursorEvent();
        virtual CursorEventPtr cloneAs(Type eventType) const;
        void setPos(const glm::vec2& pos);
        glm::vec2 getPos() const;
        int getXPosition() const;
        int getYPosition() const;
        void setCursorID(int id);
        int getCursorID() const;
        void setNode(NodePtr pNode);
        NodePtr getNode() const;
        void setSpeed(glm::vec2 speed);
        virtual const glm::vec2& getSpeed() const;

        void setContact(ContactPtr pContact);
        ContactPtr getContact() const;
        virtual void removeBlob() {};

        friend bool operator ==(const CursorEvent& event1, const CursorEvent& event2);
        virtual void trace();

    private:
        IntPoint m_Position;
        int m_ID;
        ContactWeakPtr m_pContact;
        NodePtr m_pNode;
        glm::vec2 m_Speed;
};

bool operator ==(const CursorEvent& event1, const CursorEvent& event2);

}

#endif
