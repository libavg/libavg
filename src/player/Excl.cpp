//
// $Id$
// 

#include "Excl.h"

#include "../base/Logger.h"
#include "../base/XMLHelper.h"

#include <iostream>
#include <sstream>

using namespace std;

namespace avg {

Excl::Excl ()
    : m_ActiveChild(-1)
{
}

Excl::Excl (const xmlNodePtr xmlNode, Container * pParent)
    : Container(xmlNode, pParent),
      m_ActiveChild(-1)
{
    m_ActiveChild = getDefaultedIntAttr (xmlNode, "activechild", -1);
}

Excl::~Excl ()
{
}

string Excl::dump (int indent)
{
    string dumpStr = Container::dump ();
    char sz[256];
    sprintf (sz, "m_ActiveChild= %i\n", m_ActiveChild);
    return dumpStr + string(indent+2, ' ') + sz; 
}

void Excl::render (const DRect& Rect)
{
    if (m_ActiveChild != -1) {
        getChild(m_ActiveChild)->maybeRender(Rect);
    }
}

bool Excl::obscures (const DRect& Rect, int z) 
{
    return false;
}

void Excl::getDirtyRegion (Region& Region)
{
    Node::getDirtyRegion(Region);
    if (m_ActiveChild != -1) {
        getChild(m_ActiveChild)->getDirtyRegion(Region);
    }
}

const DRect& Excl::getRelViewport()
{
    return getParent()->getRelViewport();
}

const DRect& Excl::getAbsViewport()
{
    return getParent()->getAbsViewport();
}

int Excl::getActiveChild() const
{
    return m_ActiveChild;
}

void Excl::setActiveChild(int activeChild)
{
    invalidate();
    if (activeChild < -1 || activeChild > getNumChildren()) {
        AVG_TRACE(Logger::WARNING, 
                getID() << ".setActiveChild() called with illegal value " 
                    << activeChild << ".");
    } else {
        m_ActiveChild = activeChild;
    }
    invalidate();
}

string Excl::getTypeStr ()
{
    return "Excl";
}

Node * Excl::getElementByPos (const DPoint & pos)
{
    if (m_ActiveChild != -1) {
        return getChild(m_ActiveChild)->getElementByPos(pos);
    } else {
        return 0;
    }
}

void Excl::invalidate()
{
    if (m_ActiveChild != -1) {
        addDirtyRect(getChild(m_ActiveChild)->getVisibleRect());
    }
}

}
