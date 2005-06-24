//
// $Id$
// 

#ifndef _Excl_H_
#define _Excl_H_

#include "Container.h"

#include <string>

namespace avg {

class Excl : public Container
{
    public:
        Excl ();
        Excl (const xmlNodePtr xmlNode, Container * pParent);
        virtual ~Excl ();

        virtual std::string dump (int indent = 0);
		virtual void render (const DRect& Rect);
        virtual bool obscures (const DRect& Rect, int z);
        virtual void getDirtyRegion (Region& Region);
        virtual const DRect& getRelViewport();
        virtual const DRect& getAbsViewport();

        virtual int getActiveChild() const;
        virtual void setActiveChild(int activeChild);

        std::string getTypeStr ();
        virtual Node * getElementByPos (const DPoint & pos);

    protected:
        void invalidate();

    private:
        int m_ActiveChild;
};

}

#endif //_Excl_H_

