//
// $Id$
//

#ifndef _AVGVideo_H_
#define _AVGVideo_H_

#include "AVGVisibleNode.h"

class AVGVideo : 	
	public AVGVisibleNode
{
	virtual void update (int time);
    virtual void render ();
    virtual void getDirtyRect ();
};

#endif //_AVGVideo_H_

