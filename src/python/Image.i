//
// $Id$
// 

%module avg
%{
#include "../player/Image.h"
%}

%include "RasterNode.i"

%attribute(avg::Image, const std::string&, href, getHRef);
// TODO: Hue, Saturation

namespace avg {

class Image : public RasterNode
{
	public:
        Image ();
        virtual ~Image ();
};

}

