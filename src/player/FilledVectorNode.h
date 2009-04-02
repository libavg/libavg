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

#ifndef _FilledVectorNode_H_
#define _FilledVectorNode_H_

#include "../api.h"
#include "VectorNode.h"

namespace avg {

class AVG_API FilledVectorNode : public VectorNode
{
    public:
        static NodeDefinition createDefinition();
        
        FilledVectorNode(const ArgList& Args);
        virtual ~FilledVectorNode();
        virtual void setRenderingEngines(DisplayEngine * pDisplayEngine, 
                AudioEngine * pAudioEngine);
        virtual void disconnect();
        virtual void checkReload();

        const std::string& getFillTexHRef() const;
        void setFillTexHRef(const std::string& href);
        void setFillBitmap(const Bitmap * pBmp);

        const DPoint& getFillTexCoord1() const;
        void setFillTexCoord1(const DPoint& pt);
        const DPoint& getFillTexCoord2() const;
        void setFillTexCoord2(const DPoint& pt);

        void setFillColor(const std::string& sColor);
        const std::string& getFillColor() const;

        double getFillOpacity() const;
        void setFillOpacity(double opacity);

        virtual void preRender();
        virtual void maybeRender(const DRect& Rect);
        virtual void render(const DRect& rect);

        virtual int getNumFillVertexes() = 0;
        virtual int getNumFillIndexes() = 0;
        virtual void calcFillVertexes(VertexArrayPtr& pVertexArray, Pixel32 color) = 0;

    protected:
        Pixel32 getFillColorVal() const;
        DPoint calcFillTexCoord(const DPoint& pt, const DPoint& minPt, 
                const DPoint& maxPt);

    private:
        double m_OldOpacity;

        std::string m_FillTexHRef;
        DPoint m_FillTexCoord1;
        DPoint m_FillTexCoord2;
        ShapePtr m_pFillShape;
        double m_FillOpacity;
        std::string m_sFillColorName;
        Pixel32 m_FillColor;
};

typedef boost::shared_ptr<FilledVectorNode> FilledVectorNodePtr;

}

#endif

