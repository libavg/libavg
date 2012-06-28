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
        
        CircleNode(const ArgList& args);
        virtual ~CircleNode();

        const glm::vec2& getPos() const;
        void setPos(const glm::vec2& pt);

        float getR() const;
        void setR(float r);

        float getTexCoord1() const;
        void setTexCoord1(float tc);

        float getTexCoord2() const;
        void setTexCoord2(float tc);

        void getElementsByPos(const glm::vec2& pos, std::vector<NodePtr>& pElements);
        virtual void calcVertexes(const VertexDataPtr& pVertexData, Pixel32 color);
        virtual void calcFillVertexes(const VertexDataPtr& pVertexData, Pixel32 color);

    private:
        void appendCirclePoint(const VertexDataPtr& pVertexData, const glm::vec2& iPt, 
                const glm::vec2& oPt, Pixel32 color, int& i, int& curVertex);
        void appendFillCirclePoint(const VertexDataPtr& pVertexData, 
                const glm::vec2& curPt, const glm::vec2& minPt, const glm::vec2& maxPt, 
                Pixel32 color, int& curVertex);
        int getNumCircumferencePoints();
        void getEigthCirclePoints(std::vector<glm::vec2>& pts, float radius);
        glm::vec2 getCirclePt(float angle, float radius);
        glm::vec2 calcTexCoord(const glm::vec2& pt, const glm::vec2& minPt, 
                const glm::vec2& maxPt);

        glm::vec2 m_Pos;
        float m_Radius;
        float m_TC1;
        float m_TC2;
};

}

#endif

