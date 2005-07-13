//
// $Id$
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

