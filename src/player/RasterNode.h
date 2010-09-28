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

#ifndef _RasterNode_H_
#define _RasterNode_H_

#include "../api.h"
#include "AreaNode.h"
#include "DisplayEngine.h"
#include "PBOTexture.h"
#include "FXNode.h"

#include "../avgconfigwrapper.h"
#include "../base/Point.h"
#include "../base/Triple.h"
#include "../base/UTF8String.h"
#include "../graphics/FBO.h"

#include <string>

namespace avg {

class OGLSurface;

typedef std::vector<std::vector<DPoint> > VertexGrid;

class AVG_API RasterNode: public AreaNode
{
    public:
        static NodeDefinition createDefinition();
        
        virtual ~RasterNode ();
        virtual void setRenderingEngines(DisplayEngine * pDisplayEngine, 
                AudioEngine * pAudioEngine);
        virtual void setArgs(const ArgList& Args);
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
        DisplayEngine::BlendMode getBlendMode() const;

        const UTF8String& getMaskHRef() const;
        void setMaskHRef(const UTF8String& href);

        const DPoint& getMaskPos() const;
        void setMaskPos(const DPoint& pos);

        const DPoint& getMaskSize() const;
        void setMaskSize(const DPoint& size);

        VisibleNodePtr getElementByPos(const DPoint & pos);

        DTriple getGamma() const;
        void setGamma(const DTriple& gamma);
        DTriple getIntensity() const;
        void setIntensity(const DTriple& intensity);
        DTriple getContrast() const;
        void setContrast(const DTriple& contrast);

        void setEffect(FXNodePtr pFXNode);
        virtual BitmapPtr getBitmap();
        
        void blt32(const DPoint& DestSize, double opacity, DisplayEngine::BlendMode Mode,
                bool bPremultipliedAlpha = false);
        void blta8(const DPoint& DestSize, double opacity, 
                const Pixel32& color, DisplayEngine::BlendMode Mode);

    protected:
        RasterNode();
        virtual OGLSurface * getSurface();
        const MaterialInfo& getMaterial() const;
        void setMaskCoords();
        void bind();

    private:
        void setMaterial(const MaterialInfo& material);
        virtual void calcMaskCoords(MaterialInfo& material);
        void downloadMask();
        void checkDisplayAvailable(std::string sMsg);
        void setupFX();
        void blt(const DPoint& DestSize, DisplayEngine::BlendMode Mode, 
                double opacity, const Pixel32& color, bool bPremultipliedAlpha);

        IntPoint getNumTiles();
        void calcVertexGrid(VertexGrid& grid);
        void calcTileVertex(int x, int y, DPoint& Vertex);
        void calcTexCoords();

        OGLSurface * m_pSurface;
        
        IntPoint m_MaxTileSize;
        std::string m_sBlendMode;
        DisplayEngine::BlendMode m_BlendMode;
        MaterialInfo m_Material;

        UTF8String m_sMaskHref;
        std::string m_sMaskFilename;
        BitmapPtr m_pMaskBmp;
        DPoint m_MaskPos;
        DPoint m_MaskSize;
        
        bool m_bBound;

        IntPoint m_TileSize;
        VertexGrid m_TileVertices;
        bool m_bVertexArrayDirty;
        VertexArray * m_pVertexes;
        std::vector<std::vector<DPoint> > m_TexCoords;

        DTriple m_Gamma;
        DTriple m_Intensity;
        DTriple m_Contrast;

        FBOPtr m_pFBO;
        FXNodePtr m_pFXNode;
};

}

#endif
