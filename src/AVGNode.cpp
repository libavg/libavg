//
// $Id$
// 

#include "AVGEvent.h"
#include <paintlib/plpoint.h>
#include "AVGContainer.h"

AVGNode::AVGNode (const std::string& id, AVGContainer * pParent)
    : m_ID(id),
      m_pParent(pParent)
{
}

AVGNode::AVGNode ()
{
}

AVGNode::~AVGNode()
{
}

AVGNode * AVGNode::getElementByPos (const PLPoint & pos)
{
    return 0;
}

void AVGNode::update (int time, const PLPoint& pos)
{
}

void AVGNode::render ()
{
}

void AVGNode::getDirtyRect ()
{
}

string AVGNode::dump (int indent)
{
    
    return string(indent, ' ') + getTypeStr() + ": m_ID= " + m_ID; 
}

string AVGNode::getTypeStr ()
{
    return "AVGNode";
}

const string& AVGNode::getID ()
{
    return m_ID;
}

