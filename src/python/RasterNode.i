//
// $Id$
// 

%module avg
%{
#include "../player/RasterNode.h"
%}

%include "Node.i"

namespace avg {

class RasterNode: public Node
{
    public:
        virtual ~RasterNode ();

    protected:
        RasterNode ();
};

}

