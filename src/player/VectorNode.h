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

#include "../api.h"
#include "Node.h"

#include "../graphics/Pixel32.h"

namespace avg {

class VertexArray;
typedef boost::shared_ptr<VertexArray> VertexArrayPtr;
class VertexData;
typedef boost::shared_ptr<VertexData> VertexDataPtr;

class AVG_API WideLine
{
    public:
        WideLine(const DPoint& p0, const DPoint& p1, double width);

        DPoint pt0, pt1;
        DPoint pl0, pl1;
        DPoint pr0, pr1;
        DPoint dir;
};

std::ostream& operator<<(std::ostream& os, const WideLine& line);

class AVG_API VectorNode : public Node
{
    public:
        static NodeDefinition createDefinition();
        
        VectorNode(const ArgList& Args);
        virtual ~VectorNode();
        void setRenderingEngines(DisplayEngine * pDisplayEngine, 
                AudioEngine * pAudioEngine);

        void updateData(VertexArrayPtr& pVertexArray, int curVertex, int curIndex, 
                double opacity, bool bParentDrawNeeded, bool bPosChanged);

        virtual int getNumVertexes() = 0;
        virtual int getNumIndexes() = 0;
        virtual void calcVertexes(VertexDataPtr& pVertexData, double opacity) = 0;

        void setColor(const std::string& sColor);
        const std::string& getColor() const;

        void setStrokeWidth(double width);
        double getStrokeWidth() const;

    protected:
        Pixel32 getColorVal() const;
        void updateLineData(VertexDataPtr& pVertexData, int curVertex, int curIndex,
                double opacity, const DPoint& p1, const DPoint& p2);
        bool isDrawNeeded();
        void setDrawNeeded(bool bSizeChanged);

    private:
        std::string m_sColorName;
        Pixel32 m_Color;
        double m_StrokeWidth;
        VertexDataPtr m_pVertexData;

        bool m_bDrawNeeded;
};

typedef boost::shared_ptr<VectorNode> VectorNodePtr;

}

#endif

