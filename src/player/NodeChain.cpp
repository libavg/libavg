//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2014 Ulrich von Zadow
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

#include "NodeChain.h"

#include <iostream>

using namespace std;

namespace avg {

NodeChain::NodeChain()
{
}

void NodeChain::append(const NodePtr& pNode)
{
    m_pNodes.push_back(pNode);
}

NodePtr NodeChain::getNode(int i) const
{
    return m_pNodes[i];
}

NodePtr NodeChain::getLeafNode() const
{
    return m_pNodes.front();
}

int NodeChain::getSize() const
{
    return m_pNodes.size();
}

glm::vec2 NodeChain::getCanvasPos(const glm::vec2& pos) const
{
    // Find bottom canvas node in chain
    // apply transforms from root to bottom canvas node.
    return pos;
}




}

