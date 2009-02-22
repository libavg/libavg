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
#include "../graphics/VertexArray.h"
#include "../graphics/Bitmap.h"

#include "Shape.h"

namespace avg {

class OGLSurface;

class AVG_API VectorNode : public Node
{
    public:
        static NodeDefinition createDefinition();
        
        VectorNode(const ArgList& Args, bool bIsFilled=false);
        virtual ~VectorNode();
        virtual void setRenderingEngines(DisplayEngine * pDisplayEngine, 
                AudioEngine * pAudioEngine);
        virtual void connect();
        virtual void disconnect();
        virtual void checkReload();

        const std::string& getTexHRef() const;
        void setTexHRef(const std::string& href);

        const std::string& getFillTexHRef() const;
        void setFillTexHRef(const std::string& href);

        virtual void preRender();
        virtual void maybeRender(const DRect& Rect);
        virtual void render(const DRect& rect);

        virtual int getNumVertexes() = 0;
        virtual int getNumIndexes() = 0;
        virtual int getNumFillVertexes();
        virtual int getNumFillIndexes();
        virtual void calcVertexes(VertexArrayPtr& pVertexArray, 
                VertexArrayPtr& pFillVertexArray, double opacity) = 0;

        void setColor(const std::string& sColor);
        const std::string& getColor() const;

        void setStrokeWidth(double width);
        double getStrokeWidth() const;

    protected:
        Pixel32 getColorVal() const;
        void updateLineData(VertexArrayPtr& pVertexArray, double opacity, 
                const DPoint& p1, const DPoint& p2, double TC1=0, double TC2=1);
        void setDrawNeeded(bool bSizeChanged);
        bool isDrawNeeded();
        DPoint calcTexCoord(const DPoint& origCoord);

    private:
        std::string m_sColorName;
        Pixel32 m_Color;
        double m_StrokeWidth;

        bool m_bDrawNeeded;
        bool m_bVASizeChanged;
        double m_OldOpacity;

        std::string m_TexHRef;
        ShapePtr m_pShape;
        std::string m_FillTexHRef;
        ShapePtr m_pFillShape;
};

typedef boost::shared_ptr<VectorNode> VectorNodePtr;

}

#endif

