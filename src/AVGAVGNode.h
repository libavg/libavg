//
// $Id$
// 

#ifndef _AVGAVGNode_H_
#define _AVGAVGNode_H_

#include "AVGDivNode.h"
#include "AVGPoint.h"
#include "AVGKeyEvent.h"
#include <string>

// 84d8c8d3-2af2-482a-a238-608452b93b6f
#define AVGAVGNODE_CID \
{ 0x84d8c8d3, 0x2af2, 0x482a, { 0xa2, 0x38, 0x60, 0x84, 0x52, 0xb9, 0x3b, 0x6f } }

#define AVGAVGNODE_CONTRACTID "@c-base.org/avgavgnode;1"

class AVGAVGNode : public AVGDivNode
{
	public:
        NS_DECL_ISUPPORTS_INHERITED

        static AVGAVGNode * create();
        
        AVGAVGNode ();
        virtual ~AVGAVGNode ();

        NS_IMETHOD GetType(PRInt32 *_retval);

        void initKeyEventHandlers (const std::string& sKeyDownHandler, 
                const std::string& sKeyUpHandler);
        virtual std::string getTypeStr ();
        void handleKeyEvent (AVGKeyEvent* pEvent, JSContext * pJSContext);

    private:
        std::string m_sKeyUpHandler;
        std::string m_sKeyDownHandler;

};

#endif //_AVGAVGNode_H_

