//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2014 Ulrich von Zadow
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

#include "../base/Rect.h"
#include "../base/BezierCurve.h"
#include "../graphics/Pixel32.h"

namespace avg {

// Axis-aligned bounding box.
class AVG_API CurveAABB: public FRect
{
public:
    CurveAABB(const glm::vec2& pt, int startIDX, int endIDX);
    int m_StartIdx;
    int m_EndIdx;
};
typedef std::vector<CurveAABB> CurveAABBVector;
typedef boost::shared_ptr<CurveAABBVector> CurveAABBVectorPtr;

class AVG_API CurveNode : public VectorNode
{
    public:
        static void registerType();
        
        CurveNode(const ArgList& args, const std::string& sPublisherName="Node");
        virtual ~CurveNode();

        const glm::vec2& getPos1() const;
        void setPos1(const glm::vec2& pt);

        const glm::vec2& getPos2() const;
        void setPos2(const glm::vec2& pt);

        const glm::vec2& getPos3() const;
        void setPos3(const glm::vec2& pt);

        const glm::vec2& getPos4() const;
        void setPos4(const glm::vec2& pt);

        float getTexCoord1() const;
        void setTexCoord1(float tc);

        float getTexCoord2() const;
        void setTexCoord2(float tc);

        float getCurveLen() const;
        glm::vec2 getPtOnCurve(float t) const;

        virtual void calcVertexes(const VertexDataPtr& pVertexData, Pixel32 color);

    protected:
        bool isInside(const glm::vec2& pos);

    private:
        bool isInsideBB(const glm::vec2& pos, unsigned level, unsigned i);
        void updateLines();
        void calcBoundingBoxes();
        void addCurvePoints(const glm::vec2& pos, const glm::vec2& deriv);
        BezierCurvePtr m_pCurve;
        float m_TC1;
        float m_TC2;

        std::vector<glm::vec2> m_CenterCurve;
        std::vector<glm::vec2> m_LeftCurve;
        std::vector<glm::vec2> m_RightCurve;

        // Hierarchial bounding boxes: (Bounding box levels) x (boxes in level).
        // Bounding boxes are only used for hit test calculation.
        std::vector<CurveAABBVectorPtr> m_AABBs;
};

}

#endif

