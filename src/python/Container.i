//
// $Id$
// 

%module avg
%{
#include "../player/Container.h"
%}

%include "Node.i"

namespace avg {

class Container: public Node
{
    public:
        int getNumChildren ();
        Node * getChild (int i);
        void addChild (Node * newNode, bool bInit);
        void removeChild (int i);
        int indexOf(Node * pChild);
    protected:
        Container();
};

}
