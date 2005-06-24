//
// $Id$
// 

%module avg
%{
#include "../player/AVGNode.h"
%}

%include "DivNode.i"

namespace avg {

class AVGNode : public DivNode
{
	public:
        AVGNode ();
        virtual ~AVGNode ();

        //TODO: onkeyup, onkeydown
        bool getCropSetting();
};

}

