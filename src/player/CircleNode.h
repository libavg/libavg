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

#ifndef _CircleNode_H_
#define _CircleNode_H_

#include "../api.h"
#include "FilledVectorNode.h"

#include "../graphics/Pixel32.h"

namespace avg {

class AVG_API CircleNode : public FilledVectorNode
{
    public:
        static NodeDefinition createDefinition();
        
        CircleNode(const ArgList& Args);
        virtual ~CircleNode();

        const DPoint& getPos() const;
        void setPos(const DPoint& pt);

        double getR() const;
        void setR(double r);

        double getTexCoord1() const;
        void setTexCoord1(double tc);

        double getTexCoord2() const;
        void setTexCoord2(double tc);

        VisibleNodePtr getElementByPos(const DPoint & pos);
        virtual void calcVertexes(VertexArrayPtr& pVertexArray, Pixel32 color);
        virtual void calcFillVertexes(VertexArrayPtr& pVertexArray, Pixel32 color);

    private:
        void appendCirclePoint(VertexArrayPtr& pVertexArray, const DPoint& iPt, 
                const DPoint& oPt, Pixel32 color, int& i, int& curVertex);
        void appendFillCirclePoint(VertexArrayPtr& pVertexArray, const DPoint& curPt, 
                const DPoint& minPt, const DPoint& maxPt, Pixel32 color, int& curVertex);
        int getNumCircumferencePoints();
        void getEigthCirclePoints(std::vector<DPoint>& pts, double radius);
        DPoint getCirclePt(double angle, double radius);
        DPoint calcTexCoord(const DPoint& pt, const DPoint& minPt, const DPoint& maxPt);

        DPoint m_Pos;
        double m_Radius;
        double m_TC1;
        double m_TC2;
};

}

#endif

