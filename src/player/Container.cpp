//
// $Id$
// 

#include "Container.h"
#include "Node.h"
#include "Region.h"
#include "Player.h"

#include "../base/Exception.h"

#include <iostream>
#include <sstream>

using namespace std;

namespace avg {

Container::Container ()
{
}

Container::Container (const xmlNodePtr xmlNode, Container * pParent)
    : Node(xmlNode, pParent)
{
}

Container::~Container ()
{
}

int Container::getNumChildren ()
{
    return m_Children.size();
}

Node * Container::getChild (int i)
{
    if (i >= (int)m_Children.size() || i < 0) {
        stringstream s;
        s << "Index " << i << " is out of range in Container::getChild()";
        throw(Exception(AVG_ERR_OUT_OF_RANGE, s.str()));
        
    }
    return m_Children[i];
}

void Container::addChild (Node * pNewNode)
{
    // Children are ordered according to z-position.
    vector<Node*>::iterator it;
    for  (it = m_Children.begin(); it < m_Children.end(); it++) {
        Node * pOtherNode = *it;
        if (pNewNode->getZ() < pOtherNode->getZ()) {
            break;
        }
    }
    m_Children.insert (it, pNewNode);
}

void Container::removeChild (int i)
{
    Node * pNode = getChild(i);
    pNode->invalidate();
//    JSObject * pJSNode = pNode->getJSPeer();
//    JSFactoryBase::removeParent(pJSNode, getJSPeer());
    m_Children.erase(m_Children.begin()+i);
}

int Container::indexOf(Node * pChild)
{
    for  (int i = 0; i< (int)m_Children.size(); ++i) {
        if (m_Children[i] == pChild) {
            return i;
        }
    }
    return -1;
}

void Container::zorderChange (Node * pChild)
{
    // Remove child
    vector<Node*>::iterator it;
    for  (it = m_Children.begin(); it < m_Children.end(); it++) {
        if ((*it) == pChild) {
            m_Children.erase(it);
            break;
        }
    }

    // Add it again.
    addChild(pChild);
}

void Container::prepareRender (int time, const DRect& parent)
{
    vector<Node*>::iterator it;
    for (it=m_Children.begin(); it<m_Children.end(); it++) {
        (*it)->prepareRender(time, parent);
    }
}

string Container::dump (int indent)
{
    string dumpStr = Node::dump () + "\n";
    vector<Node*>::iterator it;
    for (it=m_Children.begin(); it<m_Children.end(); it++) {
        dumpStr += (*it)->dump(indent+2)+"\n";
    }
    return dumpStr;
}

string Container::getTypeStr ()
{
    return "Container";
}

DPoint Container::getPreferredMediaSize()
{
    return DPoint(10000,10000);
}

}
