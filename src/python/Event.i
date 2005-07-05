//
// $Id$
// 

%module avg
%{
#include "../player/Event.h"
%}

namespace avg {

%attribute(avg::Event, avg::Event::Type, type, getType);
%attribute(avg::Event, int, when, getWhen);

class Event
{
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
    private:
        Event();
        
};

}
