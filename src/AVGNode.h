//
// $Id$
// 

#ifndef _AVGNode_H_
#define _AVGNode_H_

#include <vector>
#include <string>

#include <SDL/SDL.h>

class PLPoint;
class AVGContainer;
class IJSEvalKruecke;
class AVGEvent;

class AVGNode 
{
	public:
        AVGNode (const std::string& id, AVGContainer * pParent);
        AVGNode ();
        virtual ~AVGNode ();
        virtual void InitEventHandlers
            (const std::string& MouseMoveHandler, 
             const std::string& MouseButtonUpHandler, 
             const std::string& MouseButtonDownHandler,
             const std::string& MouseOverHandler, 
             const std::string& MouseOutHandler);

        virtual AVGNode * getElementByPos (const PLPoint & pos);
		virtual void update (int time, const PLPoint& pos);
		virtual void render ();
		virtual void getDirtyRect ();
        virtual std::string dump (int indent = 0);
        virtual std::string getTypeStr ();
        virtual const std::string& getID ();
        
        virtual void handleEvent (AVGEvent* pEvent, IJSEvalKruecke* pKruecke);
        virtual void onMouseOver (IJSEvalKruecke * pKruecke);
        virtual void onMouseOut (IJSEvalKruecke * pKruecke);
        virtual void callJS (const string& Code, IJSEvalKruecke * pKruecke);

	private:
        std::string m_ID;
		AVGContainer * m_pParent;

        std::string m_MouseMoveHandler;
        std::string m_MouseButtonUpHandler;
        std::string m_MouseButtonDownHandler;
        std::string m_MouseOverHandler;
        std::string m_MouseOutHandler;
};

#endif //_AVGNode_H_

