//
// $Id$
// 

#include "AVGExcl.h"
#include "AVGNode.h"

#include <nsMemory.h>
#include <xpcom/nsComponentManagerUtils.h>

#include <iostream>

using namespace std;

NS_IMPL_ISUPPORTS1_CI(AVGExcl, IAVGNode);

AVGExcl * AVGExcl::create()
{
    return createNode<AVGExcl>("@c-base.org/avgexcl;1");
}       

AVGExcl::AVGExcl ()
    : m_ActiveChild(-1)
{
}

AVGExcl::~AVGExcl ()
{
}

NS_IMETHODIMP 
AVGExcl::GetIntAttr(const char *name, PRInt32 *_retval)
{
    if (!strcmp(name, "ActiveChild")) {
        *_retval = m_ActiveChild;
    } else {
        return AVGNode::GetIntAttr(name, _retval);
    }
    return NS_OK;
}

NS_IMETHODIMP 
AVGExcl::SetIntAttr(const char *name, PRInt32 value)
{
    if (!strcmp(name, "ActiveChild")) {
        invalidate();
        m_ActiveChild = value;
        invalidate();
    } else {
        return AVGNode::SetIntAttr(name, value);
    }
    return NS_OK;
}

NS_IMETHODIMP 
AVGExcl::GetType(PRInt32 *_retval)
{
    *_retval = NT_EXCL;
    return NS_OK;
}

string AVGExcl::dump (int indent)
{
    string dumpStr = AVGContainer::dump ();
    char sz[256];
    sprintf (sz, "m_ActiveChild= %i\n", m_ActiveChild);
    return dumpStr + string(indent+2, ' ') + sz; 
}

void AVGExcl::render (const PLRect& Rect)
{
    if (m_ActiveChild != -1) {
//        cerr << "AVGExcl::render()" << endl;
        getChild(m_ActiveChild)->maybeRender(Rect);
    }
}

bool AVGExcl::obscures (const PLRect& Rect, int z) 
{
    if (m_ActiveChild != -1) {
        return getChild(m_ActiveChild)->obscures(Rect, z);
    }
    return false;
}

void AVGExcl::getDirtyRegion (AVGRegion& Region)
{
    AVGNode::getDirtyRegion(Region);
    if (m_ActiveChild != -1) {
        getChild(m_ActiveChild)->getDirtyRegion(Region);
    }
}

const PLRect& AVGExcl::getAbsViewport()
{
    if (m_ActiveChild != -1) {
        return getChild(m_ActiveChild)->getAbsViewport();
    }
    return AVGNode::getAbsViewport();
    
}

int AVGExcl::getActiveChild()
{
    return m_ActiveChild;
}

void AVGExcl::setActiveChild(int activeChild)
{
    if (getNumChildren()>activeChild) {
        m_ActiveChild = activeChild;
    } else {
        cerr << "Warning: active child of node " << getID() << " set to illegal value " 
             << activeChild << "." << endl;
    }
}

string AVGExcl::getTypeStr ()
{
    return "AVGExcl";
}

AVGNode * AVGExcl::getElementByPos (const PLPoint & pos)
{
    if (m_ActiveChild != -1) {
        return getChild(m_ActiveChild)->getElementByPos(pos);
    } else {
        return 0;
    }
}

void AVGExcl::invalidate()
{
    if (m_ActiveChild != -1) {
        addDirtyRect(getChild(m_ActiveChild)->getAbsViewport());
    }
}

bool AVGExcl::isVisibleNode()
{
    return true;
}
