//
// $Id$
// 

#ifndef _AVGExclNode_H_
#define _AVGExclNode_H_

#include "AVGContainer.h"

class AVGExclNode : 	
	public AVGContainer
{
	public:
		virtual void update (int time);
		virtual void render ();
		virtual AVGVisibleNode * getNodeByPos (const PLPoint & pos);
		virtual void getDirtyRect ();
};

#endif //_AVGExclNode_H_

