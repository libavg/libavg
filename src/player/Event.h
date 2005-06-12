//
// $Id$
//

#ifndef _Event_H_
#define _Event_H_

#include "../Object.h"

#include <functional>

namespace avg {

class Event: public Object {
    public:
        enum {
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
    
        Event();
        void init (int type, int when=-1);
        virtual ~Event();
        
        virtual void trace();
        virtual JSFactoryBase* getFactory();

        int getWhen() const;
        int getType() const;
        
        friend struct isEventAfter;

    private:
        template <class NATIVE_T>
                friend class EventFactory;

        int m_When;
        int m_Type;
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
