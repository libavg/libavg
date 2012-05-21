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

#ifndef _DivNode_H_
#define _DivNode_H_

#include "../api.h"
#include "AreaNode.h"

#include "../graphics/SubVertexArray.h"

#include "../base/UTF8String.h"

#include <string>

namespace avg {

class AVG_API DivNode : public AreaNode
{
    public:
        static NodeDefinition createDefinition();
        
        DivNode(const ArgList& args);
        virtual ~DivNode();
        virtual void connectDisplay();
        virtual void connect(CanvasPtr pCanvas);
        virtual void disconnect(bool bKill);

        unsigned getNumChildren();
        const NodePtr& getChild(unsigned i);
        void appendChild(NodePtr pNewNode);
        void insertChildBefore(NodePtr pNewNode, NodePtr pOldChild);
        void insertChildAfter(NodePtr pNewNode, NodePtr pOldChild);
        virtual void insertChild(NodePtr pNewNode, unsigned i);
        void reorderChild(NodePtr pNode, unsigned j);
        void reorderChild(unsigned i, unsigned j);
        unsigned indexOf(NodePtr pChild);
        void removeChild(NodePtr pNode);
        void removeChild(unsigned i);
        void removeChild(NodePtr pNode, bool bKill);
        void removeChild(unsigned i, bool bKill);

        virtual glm::vec2 getPivot() const;

        bool getCrop() const;
        void setCrop(bool bCrop);

        const std::string& getElementOutlineColor() const;
        void setElementOutlineColor(const std::string& sColor);

        const UTF8String& getMediaDir() const;
        void setMediaDir(const UTF8String& mediaDir);

        void getElementsByPos(const glm::vec2& pos, std::vector<NodeWeakPtr>& pElements);
        virtual void preRender(const VertexArrayPtr& pVA, bool bIsParentActive, 
                float parentEffectiveOpacity);
        virtual void render();
        virtual void renderOutlines(const VertexArrayPtr& pVA, Pixel32 color);

        virtual std::string getEffectiveMediaDir();
        virtual void checkReload();

        virtual std::string dump(int indent = 0);
        IntPoint getMediaSize();
   
    private:
        bool isChildTypeAllowed(const std::string& sType);

        UTF8String m_sMediaDir;
        bool m_bCrop;
        std::string m_sElementOutlineColor;
        Pixel32 m_ElementOutlineColor;

        SubVertexArray m_ClipVA;

        std::vector<NodePtr> m_Children;
};

}

#endif
