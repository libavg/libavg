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

#ifndef _OGLTiledSurface_H_
#define _OGLTiledSurface_H_

#include "../api.h"
#include "OGLSurface.h"
#include "DisplayEngine.h"

#include "../base/Point.h"
#include "../base/Rect.h"

#include "../graphics/OGLHelper.h"

#include <vector>
#include <string>

namespace avg {

class SDLDisplayEngine;

class AVG_API OGLTiledSurface: public OGLSurface {
    public:
        OGLTiledSurface(SDLDisplayEngine * pEngine);
        virtual ~OGLTiledSurface();

        virtual void create(const IntPoint& Size, PixelFormat PF, bool bFastDownload);

        void setTileSize(const IntPoint& tileSize);
        VertexGrid getOrigVertexCoords();
        VertexGrid getWarpedVertexCoords();
        void setWarpedVertexCoords(const VertexGrid& grid);
 
        void bind();

        void blt32(const DPoint& DestSize, double opacity, DisplayEngine::BlendMode Mode);
        void blta8(const DPoint& DestSize, double opacity, 
                const Pixel32& color, DisplayEngine::BlendMode Mode);
        void blt(const DPoint& DestSize, DisplayEngine::BlendMode Mode);

    private:
        IntPoint getNumTiles();
        void calcVertexGrid(VertexGrid& Grid);
        void calcTileVertex(int x, int y, DPoint& Vertex);
        void calcTexCoords();

        bool m_bBound;

        IntPoint m_TileSize;
        VertexGrid m_TileVertices;
        VertexGrid m_FinalVertices;
        VertexArray * m_pVertexes;
        std::vector<std::vector<DPoint> > m_TexCoords;
};

}

#endif

