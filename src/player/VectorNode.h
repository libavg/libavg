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
#include "Shape.h"

#include "../graphics/Pixel32.h"
#include "../graphics/VertexArray.h"

namespace avg {

class OGLSurface;
struct WideLine;

class AVG_API VectorNode : public Node
{
    public:
        enum LineJoin {LJ_MITER, LJ_BEVEL};

        static NodeDefinition createDefinition();
        
        VectorNode(const ArgList& Args);
        virtual ~VectorNode();
        virtual void setRenderingEngines(DisplayEngine * pDisplayEngine, 
                AudioEngine * pAudioEngine);
        virtual void connect();
        virtual void disconnect();
        virtual void checkReload();

        const std::string& getTexHRef() const;
        void setTexHRef(const std::string& href);

        virtual void preRender();
        virtual void maybeRender(const DRect& Rect);
        virtual void render(const DRect& rect);

        virtual int getNumVertexes() = 0;
        virtual int getNumIndexes() = 0;
        virtual void calcVertexes(VertexArrayPtr& pVertexArray, Pixel32 color) = 0;

        void setColor(const std::string& sColor);
        const std::string& getColor() const;

        void setStrokeWidth(double width);
        double getStrokeWidth() const;

        static LineJoin string2LineJoin(const std::string& s);
        static std::string lineJoin2String(LineJoin lineJoin);

    protected:
        Pixel32 getColorVal() const;
        void updateLineData(VertexArrayPtr& pVertexArray, Pixel32 color, 
                const DPoint& p1, const DPoint& p2, double TC1=0, double TC2=1);
        void setDrawNeeded(bool bSizeChanged);
        bool isDrawNeeded();
        bool hasVASizeChanged();
        void calcPolyLineCumulDist(std::vector<double>& cumulDist, 
                const std::vector<DPoint>& pts, bool bIsClosed);
        void calcEffPolyLineTexCoords(std::vector<double>& effTC, 
        const std::vector<double>& tc, const std::vector<double>& cumulDist);

        void calcPolyLine(const std::vector<DPoint>& origPts, 
                const std::vector<double>& origTexCoords, bool bIsClosed, LineJoin lineJoin,
                VertexArrayPtr& pVertexArray, Pixel32 color);
        void calcBevelTC(const WideLine& line1, const WideLine& line2, 
                bool bIsLeft, const std::vector<double>& texCoords, unsigned i, 
                double& TC0, double& TC1);
        int getNumDifferentPts(const std::vector<DPoint>& pts);

    private:
        std::string m_sColorName;
        Pixel32 m_Color;
        double m_StrokeWidth;

        bool m_bDrawNeeded;
        bool m_bVASizeChanged;
        double m_OldOpacity;

        std::string m_TexHRef;
        ShapePtr m_pShape;
};

typedef boost::shared_ptr<VectorNode> VectorNodePtr;

}

#endif

