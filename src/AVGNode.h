//
// $Id$
// 

#ifndef _AVGNode_H_
#define _AVGNode_H_

#include <vector>
#include <string>

class PLPoint;
class AVGContainer;

class AVGNode 
{
	public:
        AVGNode (const std::string& id, AVGContainer * pParent);
        AVGNode ();
        virtual ~AVGNode ();
		virtual AVGNode * getElementByPos (const PLPoint & pos);
		virtual void update (int time, const PLPoint& pos);
		virtual void render ();
		virtual void getDirtyRect ();
        virtual std::string dump (int indent = 0);
        virtual std::string getTypeStr ();
        virtual const std::string& getID ();

	private:
        std::string m_ID;
		AVGContainer * m_pParent;
};

#endif //_AVGNode_H_

