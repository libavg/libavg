//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2011 Ulrich von Zadow
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

#ifndef _DAG_H_
#define _DAG_H_

#include "../api.h"

#include <set>
#include <vector>
#include <boost/shared_ptr.hpp>

namespace avg {

class DAG;
class DAGNode;
typedef boost::shared_ptr<DAGNode> DAGNodePtr;

// Directed Acyclic Graph class.
// Only useful for sorting. The process of sorting destroys the DAG.
class AVG_API DAG 
{
public:
    DAG();
    virtual ~DAG();

    void addNode(long vertexID, const std::set<long>& outgoingIDs);
    void sort(std::vector<long>& pResults);

private:
    friend class DAGNode;

    void resolveIDs();
    DAGNodePtr findNode(long pID);
    void removeNode(DAGNodePtr pNode);
    DAGNodePtr findStartNode(DAGNodePtr pNode, unsigned depth=0);

    std::set<DAGNodePtr> m_pNodes;
};

}

#endif 



