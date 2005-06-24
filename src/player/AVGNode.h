//
// $Id$
// 

#ifndef _AVGNode_H_
#define _AVGNode_H_

#include "DivNode.h"
#include "Point.h"
#include "KeyEvent.h"
#include <string>

namespace avg {

class AVGNode : public DivNode
{
	public:
        AVGNode ();
        AVGNode (const xmlNodePtr xmlNode);
        virtual ~AVGNode ();

        virtual std::string getTypeStr ();

//        void handleKeyEvent (KeyEvent* pEvent);
        bool getCropSetting();

    private:
        std::string m_sKeyUpHandler;
        std::string m_sKeyDownHandler;
        bool m_bEnableCrop;

};

}

#endif //_AVGNode_H_

