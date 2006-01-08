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

#ifndef _Container_H_
#define _Container_H_

#include "Node.h"
#include <string>

namespace avg {

class Container : public Node
{
    public:
        virtual ~Container ();

        virtual void prepareRender (int time, const Rect<double>& parent);
        virtual std::string dump (int indent = 0);
        std::string getTypeStr ();
        
        int getNumChildren ();
        Node * getChild (int i);
        void addChild (Node * newNode);
        void removeChild (int i);
        int indexOf(Node * pChild);

        void zorderChange (Node * pChild);
        
    protected:        
        Container ();
        Container (const xmlNodePtr xmlNode, Container * pParent);

        virtual Point<double> getPreferredMediaSize();	
    
    private:
        std::vector <Node *> m_Children;
};

}

#endif //_Container_H_

