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

#include "OffscreenSceneNode.h"
#include "Player.h"

#include "NodeDefinition.h"

#include "../base/FileHelper.h"

using namespace std;

namespace avg {

NodeDefinition OffscreenSceneNode::createDefinition()
{
    return NodeDefinition("scene", VisibleNode::buildNode<OffscreenSceneNode>)
        .extendDefinition(SceneNode::createDefinition())
        .addArg(Arg<bool>("handleevents", false, false, 
                offsetof(OffscreenSceneNode, m_bHandleEvents)))
        .addArg(Arg<int>("multisamplesamples", 1, false, 
                offsetof(OffscreenSceneNode, m_MultiSampleSamples)))
        .addArg(Arg<bool>("mipmap", false, false, 
                offsetof(OffscreenSceneNode, m_bMipmap)));
}

OffscreenSceneNode::OffscreenSceneNode(const ArgList& Args)
    : SceneNode(Args)
{
    Args.setMembers(this);
}

OffscreenSceneNode::~OffscreenSceneNode()
{
}

bool OffscreenSceneNode::getHandleEvents() const
{
    return m_bHandleEvents;
}

bool OffscreenSceneNode::getMipmap() const
{
    return m_bMipmap;
}

int OffscreenSceneNode::getMultiSampleSamples() const
{
    return m_MultiSampleSamples;
}

}
