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

#include "../api.h"
#include "VectorNode.h"

#include "../graphics/Pixel32.h"

namespace avg {

class AVG_API CurveNode : public VectorNode
{
    public:
        static NodeDefinition createDefinition();
        
        CurveNode(const ArgList& Args, bool bFromXML);
        virtual ~CurveNode();

        const DPoint& getPos1() const;
        void setPos1(const DPoint& pt);

        const DPoint& getPos2() const;
        void setPos2(const DPoint& pt);

        const DPoint& getPos3() const;
        void setPos3(const DPoint& pt);

        const DPoint& getPos4() const;
        void setPos4(const DPoint& pt);

        double getTexCoord1() const;
        void setTexCoord1(double tc);

        double getTexCoord2() const;
        void setTexCoord2(double tc);

        virtual int getNumVertexes();
        virtual int getNumIndexes();
        virtual void calcVertexes(VertexArrayPtr& pVertexArray, Pixel32 color);

    private:
        int getCurveLen();
        void updateLines();
        void addLRCurvePoint(const DPoint& pos, const DPoint& deriv);
        DPoint m_P1;
        DPoint m_P2;
        DPoint m_P3;
        DPoint m_P4;
        double m_TC1;
        double m_TC2;

        std::vector<DPoint> m_LeftCurve;
        std::vector<DPoint> m_RightCurve;
};

}

#endif

