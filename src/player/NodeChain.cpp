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
#include "ImageNode.h"

#include "../base/Exception.h"

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

NodePtr NodeChain::getNode(unsigned i) const
{
    return m_pNodes[i];
}

NodePtr NodeChain::getLeaf() const
{
    if (m_pNodes.empty()) {
        return NodePtr();
    } else {
        return m_pNodes.front();
    }
}

bool NodeChain::empty() const
{
    return m_pNodes.empty();
}

unsigned NodeChain::getSize() const
{
    return m_pNodes.size();
}

bool NodeChain::contains(const NodePtr& pNode) const
{
    return (std::find(m_pNodes.begin(), m_pNodes.end(), pNode) != m_pNodes.end());
}

NodeChainPtr NodeChain::createPartialChain(unsigned leafIndex) const
{
    AVG_ASSERT(leafIndex < m_pNodes.size());
    NodeChainPtr pPartialChain(new NodeChain());
    for (unsigned i=leafIndex; i<m_pNodes.size(); ++i) {
        pPartialChain->append(m_pNodes[i]);
    }
    return pPartialChain;
}

glm::vec2 NodeChain::getCanvasPos(const glm::vec2& pos) const
{
    // Find bottom canvas node in chain
    unsigned i=0;
    bool bIsCanvas = false;
    while (!bIsCanvas && i<m_pNodes.size()) {
        ImageNodePtr pNode = dynamic_pointer_cast<ImageNode>(m_pNodes[i]);
        if (pNode && pNode->getSource() == GPUImage::SCENE) {
            bIsCanvas = true;
        } else {
            i++;
        }
    }
    unsigned lastCanvasNode = i;

    // Recursively apply transforms from root to bottom canvas node.
    glm::vec2 localPos = pos;
    for (int j=int(m_pNodes.size()-1); j>=int(lastCanvasNode); --j) {
        ImageNodePtr pImageNode = dynamic_pointer_cast<ImageNode>(m_pNodes[j]);
        if (pImageNode && pImageNode->getSource() == GPUImage::SCENE) {
            localPos = pImageNode->toCanvasPos(localPos);
        }
    }

    return localPos;
}

void NodeChain::dump() const
{
    for (unsigned i=0; i<m_pNodes.size(); ++i) {
        cerr << m_pNodes[i]->getTypeStr() << " ";
    }
    cerr << endl;
}


}

