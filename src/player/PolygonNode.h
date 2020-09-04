//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2020 Ulrich von Zadow
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

#ifndef _PolygonNode_H_
#define _PolygonNode_H_

#include "../api.h"
#include "FilledVectorNode.h"

#include "../graphics/Pixel32.h"
#include "../base/WideLine.h"

#include <vector>

namespace avg {

typedef std::vector<std::vector<glm::vec2> > VectorVec2Vector;

class AVG_API PolygonNode : public FilledVectorNode
{
    public:
        static void registerType();
        
        PolygonNode(const ArgList& args, const std::string& sPublisherName="Node");
        virtual ~PolygonNode();

        const std::vector<glm::vec2>& getPos() const;
        void setPos(const std::vector<glm::vec2>& pts);

        const std::vector<float>& getTexCoords() const;
        void setTexCoords(const std::vector<float>& coords);

        std::string getLineJoin() const;
        void setLineJoin(const std::string& s);

        virtual void calcVertexes(const VertexDataPtr& pVertexData, Pixel32 color);
        virtual void calcFillVertexes(const VertexDataPtr& pVertexData, Pixel32 color);

    protected:
        virtual bool isInside(const glm::vec2& pos);

    private:
        void triangulate();

        Vec2Vector m_Pts;
        Vec2Vector m_TriPts;
        std::vector<int> m_TriIndexes;

        std::vector<float> m_CumulDist;
        std::vector<float> m_TexCoords;
        std::vector<float> m_EffTexCoords;
        LineJoin m_LineJoin;

        bool m_bPtsChanged;
};

}

#endif

