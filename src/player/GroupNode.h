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

#ifndef _GroupNode_H_
#define _GroupNode_H_

#include "AreaNode.h"

#include <string>

namespace avg {
    
class GroupNode: public AreaNode
{
    public:
        static NodeDefinition getNodeDefinition();
        
        GroupNode();
        virtual ~GroupNode();
        virtual void setRenderingEngines(DisplayEngine * pDisplayEngine, 
                AudioEngine * pAudioEngine);
        virtual void connect();
        virtual void disconnect();

        bool getCrop() const;
        void setCrop(bool bCrop);

        int getNumChildren();
        NodePtr getChild(unsigned i);
        void appendChild(NodePtr pNewNode);
        void insertChildBefore(NodePtr pNewNode, NodePtr pOldChild);
        void insertChild(NodePtr pNewNode, unsigned i);
        void removeChild(NodePtr pNode);
        void removeChild(unsigned i);
        void reorderChild(NodePtr pNode, unsigned j);
        void reorderChild(unsigned i, unsigned j);
        int indexOf(NodePtr pChild);

        virtual std::string getTypeStr();

        virtual std::string dump(int indent = 0);
        IntPoint getMediaSize();
    
    private:
        std::string m_sMediaDir;
        bool m_bCrop;
        std::vector<NodePtr> m_Children;
};

}

#endif
