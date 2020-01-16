//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2014 Ulrich von Zadow
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

#include "../base/UTF8String.h"
#include "../graphics/Pixel32.h"
#include "../graphics/Color.h"
#include "../graphics/GLContext.h"

#include <boost/shared_ptr.hpp>

namespace avg {

struct WideLine;
class VertexArray;
typedef boost::shared_ptr<VertexArray> VertexArrayPtr;
class VertexData;
typedef boost::shared_ptr<VertexData> VertexDataPtr;
class Shape;
typedef boost::shared_ptr<Shape> ShapePtr;
class Bitmap;
typedef boost::shared_ptr<Bitmap> BitmapPtr;

class AVG_API VectorNode : public Node
{
    public:
        enum LineJoin {LJ_MITER, LJ_BEVEL};

        static void registerType();
        
        VectorNode(const ArgList& args, const std::string& sPublisherName="Node");
        virtual ~VectorNode();
        virtual void connectDisplay();
        virtual void connect(CanvasPtr pCanvas);
        virtual void disconnect(bool bKill);
        virtual void checkReload();

        const UTF8String& getTexHRef() const;
        void setTexHRef(const UTF8String& href);
        void setBitmap(BitmapPtr pBmp);

        const std::string& getBlendModeStr() const;
        void setBlendModeStr(const std::string& sBlendMode);

        virtual void preRender(const VertexArrayPtr& pVA, bool bIsParentActive, 
                float parentEffectiveOpacity);
        virtual void maybeRender(GLContext* pContext, const glm::mat4& parentTransform);
        virtual void render(GLContext* pContext, const glm::mat4& transform);

        void getElementsByPos(const glm::vec2& pos, NodeChainPtr& pElements);

        virtual void calcVertexes(const VertexDataPtr& pVertexData, Pixel32 color) = 0;

        void setColor(const Color& color);
        const Color& getColor() const;

        void setStrokeWidth(float width);
        float getStrokeWidth() const;

        static LineJoin string2LineJoin(const std::string& s);
        static std::string lineJoin2String(LineJoin lineJoin);

        virtual std::string dump(int indent = 0);

    protected:
        GLContext::BlendMode getBlendMode() const;

        void setDrawNeeded();
        bool isDrawNeeded();
        bool hasVASizeChanged();
        void calcPolyLineCumulDist(std::vector<float>& cumulDist, 
                const std::vector<glm::vec2>& pts, bool bIsClosed);
        void calcEffPolyLineTexCoords(std::vector<float>& effTC, 
                const std::vector<float>& tc, const std::vector<float>& cumulDist);

        void calcPolyLine(const std::vector<glm::vec2>& origPts, 
                const std::vector<float>& origTexCoords, bool bIsClosed, 
                LineJoin lineJoin, const VertexDataPtr& pVertexData, Pixel32 color);
        void calcBevelTC(const WideLine& line1, const WideLine& line2, 
                bool bIsLeft, const std::vector<float>& texCoords, unsigned i, 
                float& TC0, float& TC1);
        int getNumDifferentPts(const std::vector<glm::vec2>& pts);

        void setTranslate(const glm::vec2& trans);
        virtual bool isInside(const glm::vec2& pos);
        virtual void checkRedraw();

    private:
        Shape* createDefaultShape() const;

        Color m_Color;
        float m_StrokeWidth;
        UTF8String m_TexHRef;
        std::string m_sBlendMode;

        bool m_bDrawNeeded;
        bool m_bVASizeChanged;

        glm::vec2 m_Translate;
        ShapePtr m_pShape;
        GLContext::BlendMode m_BlendMode;
};

typedef boost::shared_ptr<VectorNode> VectorNodePtr;

}

#endif

