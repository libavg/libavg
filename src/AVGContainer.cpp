//
// $Id$
// 

#include "AVGContainer.h"
#include "AVGNode.h"
#include "AVGRegion.h"

#include <iostream>

using namespace std;

AVGContainer::AVGContainer ()
{
}

AVGContainer::~AVGContainer ()
{
    vector<AVGNode*>::iterator it;
    for (it=m_Children.begin(); it<m_Children.end(); it++) {
        NS_IF_RELEASE(*it);
    }
}

NS_IMETHODIMP 
AVGContainer::GetNumChildren(PRInt32 *_retval)
{
    *_retval = getNumChildren();
    return NS_OK;
}

NS_IMETHODIMP 
AVGContainer::GetChild(PRInt32 i, IAVGNode **_retval)
{
    IAVGNode * pChild = getChild(i);
    NS_IF_ADDREF(pChild);
    *_retval = pChild;
    return NS_OK;
}

int AVGContainer::getNumChildren ()
{
    return m_Children.size();
}

AVGNode * AVGContainer::getChild (int i)
{
    return m_Children[i];
}

void AVGContainer::addChild (AVGNode * pNewNode)
{
    // Children are ordered according to z-position.
    vector<AVGNode*>::iterator it;
    for  (it = m_Children.begin(); it < m_Children.end(); it++) {
        AVGNode * pOtherNode = *it;
        if (pNewNode->getZ() < pOtherNode->getZ()) {
            break;
        }
    }
    m_Children.insert (it, pNewNode);
}

void AVGContainer::zorderChange (AVGNode * pChild)
{
    // Remove child
    vector<AVGNode*>::iterator it;
    for  (it = m_Children.begin(); it < m_Children.end(); it++) {
        if ((*it) == pChild) {
            m_Children.erase(it);
            break;
        }
    }

    // Add it again.
    addChild(pChild);
}

void AVGContainer::prepareRender (int time, const AVGDRect& parent)
{
    vector<AVGNode*>::iterator it;
    for (it=m_Children.begin(); it<m_Children.end(); it++) {
        (*it)->prepareRender(time, parent);
    }
}

string AVGContainer::dump (int indent)
{
    string dumpStr = AVGNode::dump () + "\n";
    vector<AVGNode*>::iterator it;
    for (it=m_Children.begin(); it<m_Children.end(); it++) {
        dumpStr += (*it)->dump(indent+2)+"\n";
    }
    return dumpStr;
}

string AVGContainer::getTypeStr ()
{
    return "AVGContainer";
}

AVGDPoint AVGContainer::getPreferredMediaSize()
{
    return AVGDPoint(10000,10000);
}

