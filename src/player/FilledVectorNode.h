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

#ifndef _FilledVectorNode_H_
#define _FilledVectorNode_H_

#include "../api.h"
#include "VectorNode.h"

#include "../base/UTF8String.h"

namespace avg {

class AVG_API FilledVectorNode : public VectorNode
{
    public:
        static NodeDefinition createDefinition();
        
        FilledVectorNode(const ArgList& args);
        virtual ~FilledVectorNode();
        virtual void connectDisplay();
        virtual void disconnect(bool bKill);
        virtual void checkReload();

        const UTF8String& getFillTexHRef() const;
        void setFillTexHRef(const UTF8String& href);
        void setFillBitmap(BitmapPtr pBmp);

        const glm::vec2& getFillTexCoord1() const;
        void setFillTexCoord1(const glm::vec2& pt);
        const glm::vec2& getFillTexCoord2() const;
        void setFillTexCoord2(const glm::vec2& pt);

        void setFillColor(const std::string& sColor);
        const std::string& getFillColor() const;

        float getFillOpacity() const;
        void setFillOpacity(float opacity);

        virtual void preRender();
        virtual void render(const FRect& rect);

        virtual void calcFillVertexes(VertexArrayPtr& pVertexArray, Pixel32 color) = 0;

    protected:
        Pixel32 getFillColorVal() const;
        glm::vec2 calcFillTexCoord(const glm::vec2& pt, const glm::vec2& minPt, 
                const glm::vec2& maxPt);
        virtual bool isVisible() const;

    private:
        float m_OldOpacity;

        UTF8String m_FillTexHRef;
        glm::vec2 m_FillTexCoord1;
        glm::vec2 m_FillTexCoord2;
        ShapePtr m_pFillShape;
        float m_FillOpacity;
        std::string m_sFillColorName;
        Pixel32 m_FillColor;
};

typedef boost::shared_ptr<FilledVectorNode> FilledVectorNodePtr;

}

#endif

