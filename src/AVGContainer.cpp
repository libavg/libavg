//
// $Id$
// 

#include "AVGContainer.h"
#include "AVGNode.h"
#include "AVGVisibleNode.h"

using namespace std;

AVGContainer::AVGContainer (const std::string& id, AVGContainer * pParent)
    : AVGNode (id, pParent)
{
}

AVGContainer::~AVGContainer ()
{
    vector<AVGNode*>::iterator it;
    for (it=m_Children.begin(); it<m_Children.end(); it++) {
        delete (*it);
    }
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
    // Non-Visible children are put in front.
    vector<AVGNode*>::iterator pos;
    AVGVisibleNode * pNewVisNode = dynamic_cast<AVGVisibleNode *>(pNewNode);
    if (pNewVisNode) {
        vector<AVGNode*>::iterator it;
        for  (it = m_Children.begin(); it < m_Children.end(); it++) {
            AVGVisibleNode * pOtherVisNode = dynamic_cast<AVGVisibleNode *>(*it);
            if (pOtherVisNode && pNewVisNode->getZ() < pOtherVisNode->getZ()) {
                break;
            }
        }
        pos = it;
    } else {
      pos = m_Children.begin();
    }
    m_Children.insert (pos, pNewNode);
}

void AVGContainer::update (int time, const PLPoint& pos)
{
    vector<AVGNode*>::iterator it;
    for (it=m_Children.begin(); it<m_Children.end(); it++) {
        (*it)->update(time, pos);
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

