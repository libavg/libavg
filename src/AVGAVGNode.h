//
// $Id$
// 

#ifndef _AVGAVGNode_H_
#define _AVGAVGNode_H_

#include "AVGContainer.h"
#include <string>

class PLPoint;

//5a938e63-42c6-4218-9cbd-a7afeb4fbd36
// 84d8c8d3-2af2-482a-a238-608452b93b6f
#define AVGAVGNODE_CID \
{ 0x84d8c8d3, 0x2af2, 0x482a, { 0xa2, 0x38, 0x60, 0x84, 0x52, 0xb9, 0x3b, 0x6f } }

#define AVGAVGNODE_CONTRACTID "@c-base.org/avgavgnode;1"

class AVGAVGNode : public AVGContainer
{
	public:
        NS_DECL_ISUPPORTS_INHERITED

        static AVGAVGNode * create();
        
        AVGAVGNode ();
        virtual ~AVGAVGNode ();

        NS_IMETHOD GetType(PRInt32 *_retval);

        virtual AVGNode * getElementByPos (const PLPoint & pos);
        virtual void prepareRender (int time, const PLRect& parent);
        virtual void render (const PLRect& rect);
        virtual bool obscures (const PLRect& rect, int z);
        virtual void getDirtyRegion (AVGRegion& Region);
        virtual std::string getTypeStr ();

};

#endif //_AVGAVGNode_H_

