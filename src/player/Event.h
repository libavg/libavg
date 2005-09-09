//
// $Id$
//

#ifndef _Event_H_
#define _Event_H_

#include <functional>
#undef _POSIX_C_SOURCE

namespace avg {

class Event {
    public:
        enum Type {
            KEYUP,
            KEYDOWN,
            MOUSEMOTION,
            MOUSEBUTTONUP,
            MOUSEBUTTONDOWN,
            MOUSEOVER,  
            MOUSEOUT,
            RESIZE,
            QUIT 
        };
    
        Event(Type type, int when=-1);
        virtual ~Event();
        
        virtual void trace();

        int getWhen() const;
        Type getType() const;
        
        friend struct isEventAfter;

    private:
        int m_When;
        Type m_Type;
        int m_Counter;

        static int s_CurCounter;
};

// Functor to compare two EventPtrs chronologically
typedef Event * EventPtr;
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
