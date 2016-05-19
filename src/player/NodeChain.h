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

#ifndef _NodeChain_h_
#define _NodeChain_h_

#include "../api.h"
#include "Node.h"

#include "../base/GLMHelper.h"

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

#include <vector>

namespace avg {

class AVG_API NodeChain
{
    public:
        NodeChain();

        void append(const NodePtr& pNode);
        NodePtr getNode(int i) const;
        NodePtr getLeaf() const;
        bool empty() const;
        int getSize() const;
        bool contains(const NodePtr& pNode) const;
        glm::vec2 getRelPos(NodePtr pNode, const glm::vec2& absPos) const;

        void dump() const;

    private:
        std::vector<NodePtr> m_pNodes; // Stored leaf-first
};

typedef boost::shared_ptr<class NodeChain> NodeChainPtr;

}

#endif
