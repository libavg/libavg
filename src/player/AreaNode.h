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

#ifndef _AreaNode_H_
#define _AreaNode_H_

#include "../api.h"

#include "Node.h"

#include "../base/GLMHelper.h"
#include "../base/Rect.h"

#include "../graphics/OGLShader.h"

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

#include <string>
#include <map>

namespace avg {

class AreaNode;
class DivNode;
class ArgList;

typedef boost::shared_ptr<AreaNode> AreaNodePtr;
typedef boost::weak_ptr<AreaNode> AreaNodeWeakPtr;
typedef boost::shared_ptr<DivNode> DivNodePtr;
typedef boost::weak_ptr<DivNode> DivNodeWeakPtr;

class AVG_API AreaNode: public Node
{
    public:
        template<class NodeType>
        static NodePtr buildNode(const ArgList& args)
        {
            return NodePtr(new NodeType(args));
        }
        static NodeDefinition createDefinition();
        
        virtual ~AreaNode() = 0;
        virtual void setArgs(const ArgList& args);
        virtual void connectDisplay();
        
        float getX() const;
        void setX(float x);
        
        float getY() const;
        void setY(float Y);

        const glm::vec2& getPos() const;
        void setPos(const glm::vec2& pt);

        virtual float getWidth() const;
        virtual void setWidth(float width);
        
        virtual float getHeight() const;
        virtual void setHeight(float height);
       
        virtual glm::vec2 getSize() const;
        virtual void setSize(const glm::vec2& pt);

        float getAngle() const;
        void setAngle(float angle);
        
        virtual glm::vec2 getPivot() const;
        void setPivot(const glm::vec2& pt);
        
        virtual glm::vec2 toLocal(const glm::vec2& globalPos) const;
        virtual glm::vec2 toGlobal(const glm::vec2& localPos) const;
        
        virtual void getElementsByPos(const glm::vec2& pos, 
                std::vector<NodeWeakPtr>& pElements);

        virtual void maybeRender();
        virtual void setViewport(float x, float y, float width, float height);
        virtual const FRect& getRelViewport() const;

        virtual std::string dump(int indent = 0);
        
        virtual void checkReload() {};

        virtual IntPoint getMediaSize() 
            { return IntPoint(0,0); };

    protected:
        AreaNode();
        glm::vec2 getUserSize() const;

    private:
        FRect m_RelViewport;      // In coordinates relative to the parent.
        float m_Angle;
        glm::vec2 m_Pivot;
        bool m_bHasCustomPivot;
        
        glm::vec2 m_UserSize;
};

}

#endif

