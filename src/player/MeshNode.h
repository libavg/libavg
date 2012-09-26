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


#ifndef _MeshNode_H_
#define _MeshNode_H_

#include "../api.h"
#include "VectorNode.h"

#include "../base/GLMHelper.h"
#include "../graphics/Pixel32.h"

#include <vector>

namespace avg {

class AVG_API MeshNode : public VectorNode
{
    public:
        static void registerType();
        
        MeshNode(const ArgList& args);
        virtual ~MeshNode();
        
        void isValid(const std::vector<glm::vec2>& coords);

        const std::vector<glm::vec2>& getVertexCoords() const;
        void setVertexCoords(const std::vector<glm::vec2>& coords);

        const std::vector<glm::vec2>& getTexCoords() const;
        void setTexCoords(const std::vector<glm::vec2>& coords);

        const std::vector<glm::ivec3>& getTriangles() const;
        void setTriangles(const std::vector<glm::ivec3>& pts);

        virtual void calcVertexes(const VertexDataPtr& pVertexData, Pixel32 color);

    private:
        std::vector<glm::vec2> m_TexCoords;
        std::vector<glm::vec2> m_VertexCoords;
        std::vector<glm::ivec3> m_Triangles;
};
}
#endif

