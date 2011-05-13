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

#ifndef _AreaNode_H_
#define _AreaNode_H_

#include "../api.h"

#include "VisibleNode.h"

#include "../base/Point.h"
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

class AVG_API AreaNode: public VisibleNode
{
    public:
        template<class NodeType>
        static VisibleNodePtr buildNode(const ArgList& args)
        {
            return VisibleNodePtr(new NodeType(args));
        }
        static NodeDefinition createDefinition();
        
        virtual ~AreaNode() = 0;
        virtual void setArgs(const ArgList& args);
        virtual void setRenderingEngines(DisplayEngine * pDisplayEngine, 
                AudioEngine * pAudioEngine);
        
        double getX() const;
        void setX(double x);
        
        double getY() const;
        void setY(double Y);

        const DPoint& getPos() const;
        void setPos(const DPoint& pt);

        virtual double getWidth() const;
        virtual void setWidth(double width);
        
        virtual double getHeight() const;
        virtual void setHeight(double height);
       
        virtual DPoint getSize() const;
        virtual void setSize(const DPoint& pt);

        double getAngle() const;
        void setAngle(double angle);
        
        virtual DPoint getPivot() const;
        void setPivot(const DPoint& pt);
        
        virtual DPoint toLocal(const DPoint& pos) const;
        virtual DPoint toGlobal(const DPoint& pos) const;
        virtual void getElementsByPos(const DPoint& pos, 
                std::vector<VisibleNodeWeakPtr>& pElements);

        virtual void maybeRender(const DRect& rect);
        virtual void setViewport(double x, double y, double width, double height);
        virtual const DRect& getRelViewport() const;

        virtual std::string dump(int indent = 0);
        
        virtual void checkReload() {};

        virtual IntPoint getMediaSize() 
            { return IntPoint(0,0); };

    protected:
        AreaNode();
        DPoint getUserSize() const;

    private:
        DRect m_RelViewport;      // In coordinates relative to the parent.
        double m_Angle;
        DPoint m_Pivot;
        bool m_bHasCustomPivot;
        
        DPoint m_UserSize;
};

}

#endif

