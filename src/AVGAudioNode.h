//
//  $Id$
//

#ifndef _AVGAudioNode_H_
#define _AVGAudioNode_H_

#include "AVGNode.h"

class AVGAudioNode : 	
	public AVGNode
{
	public:
		virtual void update (int time);
		virtual void render ();

    private:
		double volume;
};


