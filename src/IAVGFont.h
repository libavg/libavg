//
// $Id$
// 

#ifndef _IAVGFONT_H_
#define _IAVGFONT_H_

#include <string>
#include <paintlib/plbitmap.h>

class IAVGFont
{
	public:
        virtual void render(PLBmp& Surface, const std::string & Text) = 0;
};

#endif 

