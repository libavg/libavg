//
// $Id$
//

#include "AVGVisibleNode.h"
#include "AVGSDLDisplayEngine.h"
#include "IJSEvalKruecke.h"

#include <paintlib/plbitmap.h>
#include <paintlib/plrect.h>
#include <paintlib/plpoint.h>

#include <stdio.h>

using namespace std;

AVGVisibleNode::AVGVisibleNode ()
{

}

AVGVisibleNode::~AVGVisibleNode()
{
}

NS_IMETHODIMP 
AVGVisibleNode::GetIntAttr(const char *name, PRInt32 *_retval)
{
    if (!strcmp(name, "Left")) {
        *_retval = m_RelViewport.tl.x;
        return NS_OK;
    } else if (!strcmp(name, "Top")) {
        *_retval = m_RelViewport.tl.y;
        return NS_OK;
    } else if (!strcmp(name, "Right")) {
        *_retval = m_RelViewport.br.x;
        return NS_OK;
    } else if (!strcmp(name, "Bottom")) {
        *_retval = m_RelViewport.br.y;
        return NS_OK;
    } else if (!strcmp(name, "Z")) {
        *_retval = m_z;
        return NS_OK;
    } else {
        return AVGNode::GetIntAttr(name, _retval);
    }

}

NS_IMETHODIMP 
AVGVisibleNode::GetFloatAttr(const char *name, float *_retval)
{
    
    return NS_OK;
}

NS_IMETHODIMP 
AVGVisibleNode::SetIntAttr(const char *name, PRInt32 value)
{
    return NS_OK;
}

NS_IMETHODIMP 
AVGVisibleNode::SetFloatAttr(const char *name, float value)
{
    return NS_OK;
}

void AVGVisibleNode::init (const string& id, int x, int y, int z, 
        int width, int height, double opacity, 
        AVGSDLDisplayEngine * pEngine, AVGContainer * pParent)
{
    AVGNode::init(id, pParent);
    m_RelViewport = PLRect(x, y, x+width, y+height);
    m_z = z;
    m_Opacity = opacity;
    m_pEngine = pEngine;
}

AVGNode * AVGVisibleNode::getElementByPos (const PLPoint & pos)
{
    if (m_AbsViewport.Contains(pos)) {
        return this;
    } else {
        return 0;
    }
}

void AVGVisibleNode::update (int time, const PLPoint& pos)
{
    m_AbsViewport = PLRect (pos+m_RelViewport.tl, pos+m_RelViewport.br);
}

string AVGVisibleNode::dump (int indent)
{
    string dumpStr = AVGNode::dump (indent);
    char sz[256];
    sprintf (sz, ", x=%i, y=%i, z=%i, width=%i, height=%i, opacity=%.2f\n",
            m_RelViewport.tl.x, m_RelViewport.tl.y, m_z, 
            m_RelViewport.Width(), m_RelViewport.Height(), m_Opacity);
    dumpStr += sz;
    sprintf (sz, "        (Abs: x=%i, y=%i, width=%i, height=%i)\n",
            m_AbsViewport.tl.x, m_AbsViewport.tl.y,  
            m_AbsViewport.Width(), m_AbsViewport.Height());
    dumpStr += sz;

    return dumpStr; 
}

void AVGVisibleNode::setViewport (int x, int y, int width, int height)
{
    PLPoint pos = m_AbsViewport.tl-m_RelViewport.tl;
    m_RelViewport = PLRect (x, y, width, height);
    m_AbsViewport = PLRect (pos+m_RelViewport.tl, pos+m_RelViewport.br);
}

const PLRect& AVGVisibleNode::getRelViewport ()
{
    return m_RelViewport;
}

const PLRect& AVGVisibleNode::getAbsViewport ()
{
    return m_AbsViewport;
}

int AVGVisibleNode::getZ ()
{
    return m_z;
}

AVGSDLDisplayEngine * AVGVisibleNode::getEngine()
{
    return m_pEngine;
}

