//
// $Id$
// 

#include "AVGExcl.h"
#include "AVGNode.h"
#include "AVGLogger.h"
#include "IAVGPlayer.h"

#include <nsMemory.h>
#include <xpcom/nsComponentManagerUtils.h>

#include <iostream>
#include <sstream>

using namespace std;

NS_IMPL_ISUPPORTS2_CI(AVGExcl, IAVGNode, IAVGExcl);

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

/* attribute long activeChild; */
NS_IMETHODIMP AVGExcl::GetActiveChild(PRInt32 *aActiveChild)
{
    *aActiveChild = m_ActiveChild;
    return NS_OK;
}

NS_IMETHODIMP AVGExcl::SetActiveChild(PRInt32 aActiveChild)
{
    setActiveChild(aActiveChild);
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
    return false;
}

void AVGExcl::getDirtyRegion (AVGRegion& Region)
{
    AVGNode::getDirtyRegion(Region);
    if (m_ActiveChild != -1) {
        getChild(m_ActiveChild)->getDirtyRegion(Region);
    }
}

const PLRect& AVGExcl::getRelViewport()
{
    if (m_ActiveChild != -1) {
        return getChild(m_ActiveChild)->getRelViewport();
    }
    return AVGNode::getRelViewport();
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
    invalidate();
    if (activeChild < -1 || activeChild > getNumChildren()) {
        AVG_TRACE(IAVGPlayer::DEBUG_ERROR, 
                getID() << ".setActiveChild() called with illegal value " 
                    << activeChild << ".");
    } else {
        m_ActiveChild = activeChild;
    }
    invalidate();
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
