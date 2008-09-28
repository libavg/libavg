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

#ifndef _VectorNode_H_
#define _VectorNode_H_

#include "Node.h"

#include "../graphics/Pixel32.h"

namespace avg {

class VertexArray;
typedef boost::shared_ptr<VertexArray> VertexArrayPtr;

class VectorNode : public Node
{
    public:
        static NodeDefinition createDefinition();
        
        VectorNode(const ArgList& Args);
        virtual ~VectorNode();
        void setRenderingEngines(DisplayEngine * pDisplayEngine, 
                AudioEngine * pAudioEngine);

        virtual int getNumTriangles() = 0;
        virtual void updateData(VertexArrayPtr pVertexArray, int triIndex, 
                double opacity, bool bParentDrawNeeded) = 0;

        void setColor(const std::string& sColor);
        const std::string& getColor() const;

        void setWidth(double d);
        double getWidth() const;

    protected:
        Pixel32 getColorVal() const;
        bool isDrawNeeded();
        void setDrawNeeded(bool bSet);

    private:
        std::string m_sColorName;
        Pixel32 m_Color;
        double m_Width;

        bool m_bDrawNeeded;
};

typedef boost::shared_ptr<VectorNode> VectorNodePtr;

}

#endif

