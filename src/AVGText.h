//
// $Id$
//

#ifndef _AVGText_H_
#define _AVGText_H_

#include "AVGVisibleNode.h"

class AVGText : 	
	public AVGVisibleNode
{
	public:
		virtual void render ();
		virtual void getDirtyRect ();
};

#endif //_AVGText_H_

