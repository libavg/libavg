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
#include "LineNode.h"
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

NodeDefinition CanvasNode::getNodeDefinition()
{
    string sChildArray[] = {"line"};
    vector<string> sChildren = vectorFromCArray(
            sizeof(sChildArray) / sizeof(*sChildArray), sChildArray); 
    return NodeDefinition("canvas", Node::buildNode<CanvasNode>)
        .extendDefinition(GroupNode::getNodeDefinition())
        .addChildren(sChildren);
}

CanvasNode::CanvasNode(const ArgList& Args, bool)
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

    m_pVertexArray = VertexArrayPtr(new VertexArray(getNumChildren(), 100));

}

void CanvasNode::disconnect()
{
    m_pVertexArray = VertexArrayPtr();
    GroupNode::disconnect();
}

void CanvasNode::preRender()
{
    int numChildren = getNumChildren();
    for (int i=0; i<numChildren; ++i) {
        dynamic_pointer_cast<LineNode>(getChild(i))->updateData(m_pVertexArray, i);
    }
    m_pVertexArray->update();
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
        int TexMode = dynamic_cast<SDLDisplayEngine*>(getDisplayEngine())->getTextureMode();
        glDisable(TexMode);
        m_pVertexArray->draw();
        glEnable(TexMode);
    }
    
    if (getCrop()) {
        getDisplayEngine()->popClipRect();
    }
}

string CanvasNode::getTypeStr() const
{
    return "canvas";
}

string CanvasNode::dump(int indent)
{
    string dumpStr = GroupNode::dump () + "\n";
    for(int i=0; i<getNumChildren(); ++i) {
        dumpStr += getChild(i)->dump(indent+2)+"\n";
    }
    return dumpStr;
}

void CanvasNode::childrenChanged()
{
    if (m_pVertexArray) {
        m_pVertexArray->changeSize(getNumChildren());
    }
}

}
