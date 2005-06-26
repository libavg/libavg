//
// $Id$
//

%module avg
%{
#include "../player/Point.h"
%}

namespace avg {

template<class NUM>
class Point
{
    public:
        Point ();
        Point (const Point<NUM>& p);
        Point (NUM X, NUM Y);
        virtual ~Point ();

        NUM x;
        NUM y;
};

%template(DPoint) Point<double>;

}
