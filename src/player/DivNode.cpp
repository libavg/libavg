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

#include "DivNode.h"
#include "DisplayEngine.h"
#include "Player.h"
#include "NodeDefinition.h"

#include "../base/Point.h"
#include "../base/Exception.h"
#include "../base/Logger.h"
#include "../base/StringHelper.h"
#include "../base/FileHelper.h"
#include "../base/MathHelper.h"

#include <iostream>
#include <sstream>

using namespace std;
using namespace boost;

namespace avg {

NodeDefinition DivNode::createDefinition()
{
    string sChildArray[] = {"image", "div", "canvas", "words", "video", "camera", 
            "panoimage", "sound"};
    vector<string> sChildren = vectorFromCArray(
            sizeof(sChildArray) / sizeof(*sChildArray), sChildArray);
    return NodeDefinition("div", Node::buildNode<DivNode>)
        .extendDefinition(GroupNode::createDefinition())
        .addChildren(sChildren)
        .addArg(Arg<string>("mediadir", "", false, offsetof(DivNode, m_sMediaDir)));
}

DivNode::DivNode(const ArgList& Args, bool)
{
    Args.setMembers(this);
}

DivNode::~DivNode()
{
}

const string& DivNode::getMediaDir() const
{
    return m_sMediaDir;
}

void DivNode::setMediaDir(const string& sMediaDir)
{
    m_sMediaDir = sMediaDir;
    checkReload();
}

AreaNodePtr DivNode::getElementByPos (const DPoint & pos)
{
    DPoint relPos = toLocal(pos);
    if (relPos.x >= 0 && relPos.y >= 0 && 
            relPos.x < getSize().x && relPos.y < getSize().y &&
            reactsToMouseEvents())
    {
        for (int i=getNumChildren()-1; i>=0; i--) {
            AreaNodePtr pFoundNode = dynamic_pointer_cast<AreaNode>(getChild(i))
                    ->getElementByPos(relPos);
            if (pFoundNode) {
                return pFoundNode;
            }
        }
        // Pos isn't in any of the children.
        if (getSize() != DPoint(10000, 10000)) {
            // Explicit width/height given for div.
            return dynamic_pointer_cast<AreaNode>(getThis());
        } else {
            // Explicit width/height not given: div itself doesn't react.
            return AreaNodePtr();
        }
    } else { 
        return AreaNodePtr();
    }
}

void DivNode::preRender()
{
    for (int i=0; i<getNumChildren(); i++) {
        getChild(i)->preRender();
    }
}

void DivNode::render(const DRect& rect)
{
    DPoint Viewport = getSize();
    if (getCrop()) {
        DRect ClipRect(0, 0, Viewport.x, Viewport.y);
        getDisplayEngine()->pushClipRect(ClipRect);
    }
    for (int i=0; i<getNumChildren(); i++) {
        getDisplayEngine()->pushShader();
        getChild(i)->maybeRender(rect);
        getDisplayEngine()->popShader();
    }
    if (getCrop()) {
        getDisplayEngine()->popClipRect();
    }
}

string DivNode::getEffectiveMediaDir()
{
    string sMediaDir = m_sMediaDir;
    if (!isAbsPath(sMediaDir)) {
        if (getDivParent()) {
            sMediaDir = getDivParent()->getEffectiveMediaDir()+m_sMediaDir;
        } else {
            sMediaDir = Player::get()->getRootMediaDir()+m_sMediaDir;
        }
    }
    if (sMediaDir[sMediaDir.length()-1] != '/') {
        sMediaDir += '/';
    }
    return sMediaDir;
}

void DivNode::checkReload()
{
    for(int i=0; i<getNumChildren(); ++i) {
        getChild(i)->checkReload();
    }
}

string DivNode::dump(int indent)
{
    string dumpStr = GroupNode::dump () + "\n";
    for(int i=0; i<getNumChildren(); ++i) {
        dumpStr += getChild(i)->dump(indent+2)+"\n";
    }
    return dumpStr;
}

}
