//
// $Id$
// 

//
// $Id$
// 

%module avg
%{
#include "../player/DivNode.h"
%}

%include "Container.i"

namespace avg {
    
class DivNode : public Container
{
	public:
        DivNode ();
        virtual ~DivNode ();
};

}

