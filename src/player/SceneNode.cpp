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

#include "SceneNode.h"
#include "Player.h"

#include "NodeDefinition.h"

#include "../base/FileHelper.h"

using namespace std;

namespace avg {

NodeDefinition CanvasNode::createDefinition()
{
    return NodeDefinition("canvasbase", Node::buildNode<CanvasNode>)
        .extendDefinition(DivNode::createDefinition());
}

CanvasNode::CanvasNode(const ArgList& Args)
    : DivNode(Args)
{
    Args.setMembers(this);
}

CanvasNode::~CanvasNode()
{
}

string CanvasNode::getEffectiveMediaDir()
{
    string sMediaDir = getMediaDir();
    if (!isAbsPath(sMediaDir)) {
        sMediaDir = Player::get()->getCurDirName()+sMediaDir;
    }
    if (sMediaDir[sMediaDir.length()-1] != '/') {
        sMediaDir += '/';
    }
    return sMediaDir;
}

}
