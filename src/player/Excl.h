//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2006 Ulrich von Zadow
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  Current versions can be found at www.libavg.de
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
        virtual const DRect& getRelViewport() const;
        virtual const DRect& getAbsViewport() const;

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

