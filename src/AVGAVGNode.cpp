//
// $Id$
// 

#include "AVGAVGNode.h"
#include "AVGContainer.h"
#include "AVGVisibleNode.h"
#include "AVGSDLDisplayEngine.h"

#include <paintlib/plpoint.h>

AVGAVGNode::AVGAVGNode(const std::string& id, int x, int y, int z, 
           int width, int height, double opacity, 
           AVGSDLDisplayEngine * pEngine, AVGContainer * pParent)
    : AVGContainer(id, pParent),
      AVGVisibleNode(id, x, y, z, width, height, opacity, pEngine, pParent),
      AVGNode(id, pParent)
{
}

AVGAVGNode::~AVGAVGNode()
{
}

AVGNode * AVGAVGNode::getElementByPos (const PLPoint & pos)
{
    if (!getAbsViewport().Contains(pos)) {
        return 0; // Not in parent.
    }
    for (int i=getNumChildren()-1; i>=0; i--) {
        AVGNode * pFoundNode = getChild(i)->getElementByPos(pos);
        if (pFoundNode) {
            return pFoundNode;
        }
    }
    return this; // In parent, but not in any child.
}

void AVGAVGNode::update (int time, const PLPoint& pos)
{
    AVGVisibleNode::update(time, pos);
    for (int i=0; i<getNumChildren(); i++){
        getChild(i)->update(time, getAbsViewport().tl);
    }
}

void AVGAVGNode::render ()
{
    getEngine()->setClipRect(getAbsViewport());
    for (int i=0; i<getNumChildren(); i++){
        getChild(i)->render();
    }
}

void AVGAVGNode::getDirtyRect ()
{

}

string AVGAVGNode::dump (int indent)
{
    // Messy duplicated code because of multiple inheritance.
    string dumpStr = AVGVisibleNode::dump(indent);
    for (int i=0; i<getNumChildren(); i++){
        dumpStr += getChild(i)->dump(indent+2);
    }

    return dumpStr;
}

string AVGAVGNode::getTypeStr ()
{
    return "AVGAVGNode";
}

