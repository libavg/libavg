//
// $Id$
// 

#ifndef _AVGExcl_H_
#define _AVGExcl_H_

#include "AVGContainer.h"
#include <string>

// 1d2e8ea7-2d53-4bf5-9225-bae68091754e
#define AVGEXCL_CID \
{ 0x1d2e8ea7, 0x2d53, 0x4bf5, { 0x92, 0x25, 0xba, 0xe6, 0x80, 0x91, 0x75, 0x4e } }

#define AVGEXCL_CONTRACTID "@c-base.org/avgexcl;1"

class AVGExcl :	public AVGContainer
{
    public:
        NS_DECL_ISUPPORTS

        static AVGExcl * create();

        AVGExcl ();
        virtual ~AVGExcl ();

        NS_IMETHOD GetIntAttr(const char *name, PRInt32 *_retval); 
        NS_IMETHOD SetIntAttr(const char *name, PRInt32 value); 
        NS_IMETHOD GetType(PRInt32 *_retval);

        virtual std::string dump (int indent = 0);
		virtual void render (const PLRect& Rect);
        virtual bool obscures (const PLRect& Rect, int z);
        virtual void getDirtyRegion (AVGRegion& Region);
        virtual const PLRect& getAbsViewport();

        virtual int getActiveChild();
        virtual void setActiveChild(int activeChild);

        std::string getTypeStr ();
        virtual AVGNode * getElementByPos (const PLPoint & pos);

    protected:
        void invalidate();
        virtual bool isVisibleNode();  // Poor man's RTTI

    private:
        int m_ActiveChild;
};

#endif //_AVGExcl_H_

