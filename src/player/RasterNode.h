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

#ifndef _RasterNode_H_
#define _RasterNode_H_

#include "../api.h"
#include "AreaNode.h"
#include "MaterialInfo.h"

#include "../avgconfigwrapper.h"
#include "../base/GLMHelper.h"
#include "../base/UTF8String.h"
#include "../graphics/GLContext.h"
#include "../graphics/SubVertexArray.h"

#include <string>

namespace avg {

class OGLSurface;
class ImagingProjection;
typedef boost::shared_ptr<ImagingProjection> ImagingProjectionPtr;
class MCFBO;
typedef boost::shared_ptr<MCFBO> MCFBOPtr;
class FXNode;
typedef boost::shared_ptr<FXNode> FXNodePtr;

typedef std::vector<std::vector<glm::vec2> > VertexGrid;

class AVG_API RasterNode: public AreaNode
{
    public:
        static void registerType();
        
        virtual ~RasterNode ();
        virtual void connectDisplay();
        virtual void setArgs(const ArgList& args);
        virtual void disconnect(bool bKill);
        virtual void checkReload();

        // Warping support.
        VertexGrid getOrigVertexCoords();
        VertexGrid getWarpedVertexCoords();
        void setWarpedVertexCoords(const VertexGrid& Grid);

        int getMaxTileWidth() const;
        int getMaxTileHeight() const;
        bool getMipmap() const;
        
        const std::string& getBlendModeStr() const;
        void setBlendModeStr(const std::string& sBlendMode);
        GLContext::BlendMode getBlendMode() const;

        const UTF8String& getMaskHRef() const;
        void setMaskHRef(const UTF8String& sHref);

        const glm::vec2& getMaskPos() const;
        void setMaskPos(const glm::vec2& pos);

        const glm::vec2& getMaskSize() const;
        void setMaskSize(const glm::vec2& size);

        void getElementsByPos(const glm::vec2& pos, std::vector<NodePtr>& pElements);

        glm::vec3 getGamma() const;
        void setGamma(const glm::vec3& gamma);
        glm::vec3 getIntensity() const;
        void setIntensity(const glm::vec3& intensity);
        glm::vec3 getContrast() const;
        void setContrast(const glm::vec3& contrast);

        void setEffect(FXNodePtr pFXNode);
        virtual void renderFX();
        void resetFXDirty();

    protected:
        RasterNode();
        
        void scheduleFXRender();
        void calcVertexArray(const VertexArrayPtr& pVA);
        void blt32();
        void blta8(const glm::mat4& transform, const glm::vec2& destSize);

        virtual OGLSurface * getSurface();
        const MaterialInfo& getMaterial() const;
        bool hasMask() const;
        void setMaskCoords();
        void setRenderColor(const Pixel32& color);

        void newSurface();
        void setupFX();

    private:
        void downloadMask();
        virtual void calcMaskCoords();
        void checkDisplayAvailable(std::string sMsg);
        void blt(const glm::mat4& transform, const glm::vec2& destSize);

        IntPoint getNumTiles();
        void calcVertexGrid(VertexGrid& grid);
        void calcTileVertex(int x, int y, glm::vec2& Vertex);
        void calcTexCoords();

        OGLSurface * m_pSurface;
        
        IntPoint m_MaxTileSize;
        std::string m_sBlendMode;
        GLContext::BlendMode m_BlendMode;
        MaterialInfo m_Material;

        UTF8String m_sMaskHref;
        std::string m_sMaskFilename;
        BitmapPtr m_pMaskBmp;
        glm::vec2 m_MaskPos;
        glm::vec2 m_MaskSize;
        Pixel32 m_Color;

        IntPoint m_TileSize;
        VertexGrid m_TileVertices;
        SubVertexArray m_SubVA;
        std::vector<std::vector<glm::vec2> > m_TexCoords;

        glm::vec3 m_Gamma;
        glm::vec3 m_Intensity;
        glm::vec3 m_Contrast;

        MCFBOPtr m_pFBO;
        FXNodePtr m_pFXNode;
        bool m_bFXDirty;
        ImagingProjectionPtr m_pImagingProjection;
};

}

#endif
