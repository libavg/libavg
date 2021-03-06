//
//  libavg - Media Playback Engine.
//  Copyright (C) 2003-2021 Ulrich von Zadow
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

#ifndef _CursorState_H_
#define _CursorState_H_

#include "../api.h"

#include <vector>
#include <boost/shared_ptr.hpp>

namespace avg {

class Node;
typedef boost::shared_ptr<Node> NodePtr;
class NodeChain;
typedef boost::shared_ptr<NodeChain> NodeChainPtr;
class CursorEvent;
typedef boost::shared_ptr<CursorEvent> CursorEventPtr;

class AVG_API CursorState {
public:
    CursorState(const CursorEventPtr pEvent, NodeChainPtr pNodes);
    ~CursorState();

    const NodeChainPtr& getNodes() const;
    CursorEventPtr getLastEvent() const;

private:
    CursorState(const CursorState&);

    NodeChainPtr m_pNodes;
    CursorEventPtr m_pLastEvent;
};

typedef boost::shared_ptr<CursorState> CursorStatePtr;

}
#endif
