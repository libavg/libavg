//
// $Id$
// 

#ifndef _AVGContainer_H_
#define _AVGContainer_H_

#include "AVGNode.h"
#include <string>

class AVGContainer : 	
	public virtual AVGNode
{
    public:
        AVGContainer ();
        virtual ~AVGContainer ();

        NS_IMETHOD GetNumChildren(PRInt32 *_retval);
        NS_IMETHOD GetChild(PRInt32 i, IAVGNode **_retval);

		virtual void prepareRender (int time, const PLRect& parent);
        virtual std::string dump (int indent = 0);
        std::string getTypeStr ();
        
        int getNumChildren ();
        AVGNode * getChild (int i);
        void addChild (AVGNode * newNode);

        void zorderChange (AVGNode * pChild);
        
	private:
        std::vector < AVGNode * > m_Children;
};

#endif //_AVGContainer_H_

