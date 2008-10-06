//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2008 Ulrich von Zadow
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  Current versions can be found at www.libavg.de
//

#include "CanvasNode.h"
#include "VectorNode.h"
#include "SDLDisplayEngine.h"
#include "Player.h"
#include "NodeDefinition.h"

#include "../base/Point.h"
#include "../base/Exception.h"
#include "../base/Logger.h"
#include "../base/MathHelper.h"
#include "../base/ScopeTimer.h"

#include <iostream>
#include <sstream>

using namespace std;
using namespace boost;

namespace avg {

NodeDefinition CanvasNode::createDefinition()
{
    string sChildArray[] = {"line", "rect", "curve"};
    vector<string> sChildren = vectorFromCArray(
            sizeof(sChildArray) / sizeof(*sChildArray), sChildArray); 
    return NodeDefinition("canvas", Node::buildNode<CanvasNode>)
        .extendDefinition(GroupNode::createDefinition())
        .addChildren(sChildren);
}

CanvasNode::CanvasNode(const ArgList& Args, bool)
    : m_LastOpacity(-1)
{
    Args.setMembers(this);
}

CanvasNode::~CanvasNode()
{
}

void CanvasNode::setRenderingEngines(DisplayEngine * pDisplayEngine, 
        AudioEngine * pAudioEngine)
{
    GroupNode::setRenderingEngines(pDisplayEngine, pAudioEngine);

    m_pVertexArray = VertexArrayPtr(new VertexArray(getNumVertexes(), getNumIndexes(),
            100, 100));
    m_bChildrenChanged = false;
}

void CanvasNode::disconnect()
{
    m_pVertexArray = VertexArrayPtr();
    GroupNode::disconnect();
}

static ProfilingZone PrerenderProfilingZone("CanvasNode::prerender");
static ProfilingZone VAProfilingZone("CanvasNode::update VA");
static ProfilingZone VASizeProfilingZone("CanvasNode::resize VA");

void CanvasNode::preRender()
{
    ScopeTimer Timer(PrerenderProfilingZone);

    if (m_bChildrenChanged) {
        ScopeTimer Timer(VASizeProfilingZone);
        m_pVertexArray->changeSize(getNumVertexes(), getNumIndexes());
    }
    int numChildren = getNumChildren();
    int curVertex = 0;
    int curIndex = 0;
    double opacity = getEffectiveOpacity();
    bool bUpdateEverything = m_LastOpacity != opacity;
    for (int i=0; i<numChildren; ++i) {
        VectorNode * pVector = getCanvasChild(i);
        pVector->updateData(m_pVertexArray, curVertex, curIndex, opacity, 
                bUpdateEverything);
        curVertex += pVector->getNumVertexes();
        curIndex += pVector->getNumIndexes();
    }
    {
        ScopeTimer Timer(VAProfilingZone);
        m_pVertexArray->update();
    }
}

static ProfilingZone RenderProfilingZone("CanvasNode::render");

void CanvasNode::render(const DRect& rect)
{
    DPoint Viewport = getSize();
    if (getCrop()) {
        DRect ClipRect(0, 0, Viewport.x, Viewport.y);
        getDisplayEngine()->pushClipRect(ClipRect);
    }
    {
        ScopeTimer Timer(RenderProfilingZone);
        int TexMode = dynamic_cast<SDLDisplayEngine*>(
                getDisplayEngine())->getTextureMode();
        glDisable(TexMode);
        glEnableClientState(GL_COLOR_ARRAY);
        m_pVertexArray->draw();
        glEnable(TexMode);
        glDisableClientState(GL_COLOR_ARRAY);
    }
    
    if (getCrop()) {
        getDisplayEngine()->popClipRect();
    }
}

string CanvasNode::dump(int indent)
{
    string dumpStr = GroupNode::dump () + "\n";
    for(int i=0; i<getNumChildren(); ++i) {
        dumpStr += getChild(i)->dump(indent+2)+"\n";
    }
    return dumpStr;
}

VectorNode * CanvasNode::getCanvasChild(int i)
{
    // Can't use dynamic_cast for speed reasons.
    return (VectorNode *)(&*getChild(i));
}

void CanvasNode::childrenChanged()
{
    m_bChildrenChanged = true;
}

int CanvasNode::getNumVertexes()
{
    int numVertexes = 0;
    for (int i=0; i<getNumChildren(); i++) {
        numVertexes += getCanvasChild(i)->getNumVertexes();
    }
    return numVertexes;
}

int CanvasNode::getNumIndexes()
{
    int numIndexes = 0;
    for (int i=0; i<getNumChildren(); i++) {
        numIndexes += getCanvasChild(i)->getNumIndexes();
    }
    return numIndexes;
}

}
