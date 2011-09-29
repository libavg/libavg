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

#ifndef _Node_H_
#define _Node_H_

#include "../api.h"
#include "../graphics/VertexArray.h"

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

#include <string>
#include <vector>

namespace avg {

class Node;
typedef boost::shared_ptr<Node> NodePtr;
typedef boost::weak_ptr<Node> NodeWeakPtr;
class DivNode;
typedef boost::shared_ptr<DivNode> DivNodePtr;
typedef boost::weak_ptr<DivNode> DivNodeWeakPtr;
class ArgList;
class NodeDefinition;


class AVG_API Node
{
    public:
        template<class NodeType>
        static NodePtr buildNode(const ArgList& Args)
        {
            return NodePtr(new NodeType(Args));
        }
        static NodeDefinition createDefinition();
        
        virtual ~Node() = 0;
        virtual void setThis(NodeWeakPtr This, const NodeDefinition * pDefinition);
        virtual void setArgs(const ArgList& Args) {};

        void checkSetParentError(DivNodeWeakPtr pParent);
        virtual void setParent(DivNodeWeakPtr pParent);
        DivNodePtr getParent() const;

        virtual const std::string& getID() const;
        virtual void setID(const std::string& id);

        std::string getTypeStr() const;
        
        bool operator ==(const Node& other) const;
        bool operator !=(const Node& other) const;
        long getHash() const;

        virtual const NodeDefinition* getDefinition() const;
        virtual std::string dump(int indent = 0) = 0;

        virtual void preRender() = 0;
        virtual void renderOutlines(const VertexArrayPtr& pVA, Pixel32 color) {};

    protected:
        Node();
        NodePtr getThis() const;

    private:
        NodeWeakPtr m_This;
        std::string m_ID;
        const NodeDefinition* m_pDefinition;

        DivNodeWeakPtr m_pParent;
};

}

#endif
