//
// $Id$
// 

#ifndef _AVGDivNode_H_
#define _AVGDivNode_H_

#include "AVGContainer.h"
#include "AVGPoint.h"
#include <string>

// 94e6e8ac-3d2b-4635-8e1a-f321afd9c9e1
#define AVGDIVNODE_CID \
{ 0x94e6e8ac, 0x3d2b, 0x4635, { 0x8e, 0x1a, 0xf3, 0x21, 0xaf, 0xd9, 0xc9, 0xe1 } }
#define AVGDIVNODE_CONTRACTID "@c-base.org/avgdivnode;1"

class AVGDivNode : public AVGContainer
{
	public:
        NS_DECL_ISUPPORTS_INHERITED

        static AVGDivNode * create();
        
        AVGDivNode ();
        virtual ~AVGDivNode ();

        NS_IMETHOD GetType(PRInt32 *_retval);

        virtual AVGNode * getElementByPos (const AVGPoint<double> & pos);
        virtual void prepareRender (int time, const AVGRect<double>& parent);
        virtual void render (const AVGRect<double>& rect);
        virtual bool obscures (const AVGRect<double>& rect, int z);
        virtual void getDirtyRegion (AVGRegion& Region);
        virtual std::string getTypeStr ();

};

#endif //_AVGDivNode_H_

