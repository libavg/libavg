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

#ifndef _CurveNode_H_
#define _CurveNode_H_

#include "VectorNode.h"

#include "../graphics/Pixel32.h"

namespace avg {

class VertexArray;
typedef boost::shared_ptr<VertexArray> VertexArrayPtr;

class CurveNode : public VectorNode
{
    public:
        static NodeDefinition createDefinition();
        
        CurveNode(const ArgList& Args, bool bFromXML);
        virtual ~CurveNode();

        void setRenderingEngines(DisplayEngine * pDisplayEngine, 
            AudioEngine * pAudioEngine);

        double getX1() const;
        void setX1(double x);
        
        double getY1() const;
        void setY1(double y);

        const DPoint& getPos1() const;
        void setPos1(const DPoint& pt);

        double getX2() const;
        void setX2(double x);
        
        double getY2() const;
        void setY2(double y);

        const DPoint& getPos2() const;
        void setPos2(const DPoint& pt);

        double getX3() const;
        void setX3(double x);
        
        double getY3() const;
        void setY3(double y);

        const DPoint& getPos3() const;
        void setPos3(const DPoint& pt);

        double getX4() const;
        void setX4(double x);
        
        double getY4() const;
        void setY4(double y);

        const DPoint& getPos4() const;
        void setPos4(const DPoint& pt);

        virtual int getNumVertexes();
        virtual int getNumIndexes();
        virtual void updateData(VertexArrayPtr& pVertexArray, int curVertex, int curIndex, 
                double opacity, bool bParentDrawNeeded);

    private:
        int getCurveLen();
        void updateLines();
        void addLRCurvePoint(const DPoint& pos, const DPoint& deriv);
        DPoint m_P1;
        DPoint m_P2;
        DPoint m_P3;
        DPoint m_P4;

        std::vector<DPoint> m_LeftCurve;
        std::vector<DPoint> m_RightCurve;
};

}

#endif

