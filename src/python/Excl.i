//
// $Id$
// 

%module avg
%{
#include "../player/Excl.h"
%}

%attribute(avg::Excl, int, activechild, getActiveChild, setActiveChild);

namespace avg {

class Excl
{
};

}

