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

#ifndef _Event_H_
#define _Event_H_

#include <functional>
#include <boost/shared_ptr.hpp>
#undef _POSIX_C_SOURCE

namespace avg {

class Node;
typedef boost::shared_ptr<class Node> NodePtr;
class Event {
    public:
        enum Type {
            KEYUP,
            KEYDOWN,
            CURSORMOTION,
            CURSORUP,
            CURSORDOWN,
            CURSOROVER,  
            CURSOROUT,
            RESIZE,
            QUIT 
        };
        enum Source {MOUSE=1, TOUCH=2, TRACK=4, NONE=8};
    
        Event(Type type, Source source=NONE, int when=-1);
        Event(const Event& e);
        virtual ~Event();
        
        virtual void trace();

        long long getWhen() const;
        Type getType() const;
        Event::Source getSource() const;
        NodePtr getElement() const;
        void setElement(NodePtr pNode);
        
        friend struct isEventAfter;
        
    protected:
        Type m_Type;
        NodePtr m_pNode;
        long long m_When;

    private:
        int m_Counter;
        Source m_Source;

        static int s_CurCounter;
};

// Functor to compare two EventPtrs chronologically
typedef boost::shared_ptr<class Event> EventPtr;
struct isEventAfter:std::binary_function<EventPtr, EventPtr, bool> {
    bool operator()(const EventPtr & x, const EventPtr & y) const {
        if (x->getWhen() == y->getWhen()) {
            return x->m_Counter > y->m_Counter;
        }
        return x->getWhen() > y->getWhen();
    }
};

}
#endif //_Event_H_
