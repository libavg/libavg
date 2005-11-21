//
// $Id$
// 

#ifndef _DivNode_H_
#define _DivNode_H_

#include "Container.h"
#include "../graphics/Point.h"
#include <string>

namespace avg {
    
class DivNode : public Container
{
	public:
        DivNode ();
        DivNode (const xmlNodePtr xmlNode, Container * pParent);
        virtual ~DivNode ();
        virtual void init(DisplayEngine * pEngine, Container * pParent, 
                Player * pPlayer);

        virtual Node * getElementByPos (const DPoint & pos);
        virtual void prepareRender (int time, const DRect& parent);
        virtual void render (const DRect& rect);
        virtual bool obscures (const DRect& rect, int z);
        virtual void getDirtyRegion (Region& Dirtyregion);
        virtual std::string getTypeStr ();

};

}

#endif //_DivNode_H_

