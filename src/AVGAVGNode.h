//
// $Id$
// 

#ifndef _AVGAVGNode_H_
#define _AVGAVGNode_H_

#include "AVGContainer.h"
#include "AVGVisibleNode.h"
#include <string>

class AVGSDLDisplayEngine;
class PLPoint;

class AVGAVGNode : 	
	public AVGContainer, 
	public AVGVisibleNode
{
	public:
        AVGAVGNode (const std::string& id, int x, int y, int z, 
                int width, int height, double opacity, 
                AVGSDLDisplayEngine * pEngine, AVGContainer * pParent);
        virtual ~AVGAVGNode ();
		virtual AVGVisibleNode * getNodeByPos (const PLPoint & pos);
		virtual void update (int time, const PLPoint& pos);
		virtual void render ();
		virtual void getDirtyRect ();
        virtual std::string dump (int indent = 0);
        virtual std::string getTypeStr ();
};

#endif //_AVGAVGNode_H_

