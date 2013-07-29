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

#ifndef _RectNode_H_
#define _RectNode_H_

#include "../api.h"
#include "FilledVectorNode.h"

#include "../graphics/Pixel32.h"

namespace avg {

class AVG_API RectNode : public FilledVectorNode
{
    public:
        static void registerType();
        
        RectNode(const ArgList& args);
        virtual ~RectNode();

        const glm::vec2& getPos() const;
        void setPos(const glm::vec2& pt);

        glm::vec2 getSize() const;
        void setSize(const glm::vec2& pt);

        const std::vector<float>& getTexCoords() const;
        void setTexCoords(const std::vector<float>& coords);

        float getAngle() const;
        void setAngle(float angle);

        glm::vec2 toLocal(const glm::vec2& globalPos) const;
        glm::vec2 toGlobal(const glm::vec2& localPos) const;
        void getElementsByPos(const glm::vec2& pos, std::vector<NodePtr>& pElements);

        virtual void calcVertexes(const VertexDataPtr& pVertexData, Pixel32 color);
        virtual void calcFillVertexes(const VertexDataPtr& pVertexData, Pixel32 color);

    private:
        FRect m_Rect;
        std::vector<float> m_TexCoords;

        float m_Angle;
};

}

#endif

