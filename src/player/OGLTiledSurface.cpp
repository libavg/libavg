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

#include "OGLTiledSurface.h"
#include "Player.h"
#include "SDLDisplayEngine.h"
#include "MaterialInfo.h"

#include "../base/MathHelper.h"
#include "../base/Logger.h"
#include "../base/Exception.h"
#include "../base/ScopeTimer.h"
#include "../base/ObjectCounter.h"
#include "../base/Point.h"

#include <iostream>
#include <sstream>

using namespace std;

namespace avg {

OGLTiledSurface::OGLTiledSurface(const MaterialInfo& material)
    : OGLSurface(material),
      m_bBound(false),
      m_TileSize(-1,-1),
      m_pVertexes(0)
{
    ObjectCounter::get()->incRef(&typeid(*this));
}

OGLTiledSurface::~OGLTiledSurface()
{
    ObjectCounter::get()->decRef(&typeid(*this));
}

void OGLTiledSurface::create(const IntPoint& Size, PixelFormat pf)
{
    if (m_bBound && getSize() == Size && getPixelFormat() == pf) {
        // If nothing's changed, we can ignore everything.
        return;
    }
    m_bBound = false;
    OGLSurface::create(Size, pf);
       
    calcVertexGrid(m_TileVertices);
}

void OGLTiledSurface::destroy()
{
    if (m_pVertexes) {
        delete m_pVertexes;
        m_pVertexes = 0;
    }
    m_bBound = false;
    OGLSurface::destroy();
}

void OGLTiledSurface::setTileSize(const IntPoint& tileSize)
{
    m_bBound = false;
    m_TileSize = tileSize;
    calcVertexGrid(m_TileVertices);
}

VertexGrid OGLTiledSurface::getOrigVertexCoords()
{
    if (!m_bBound) {
        bind();
    }
    VertexGrid grid;
    calcVertexGrid(grid);
    return grid;
}

VertexGrid OGLTiledSurface::getWarpedVertexCoords()
{
    if (!m_bBound) {
        bind();
    }
    return m_TileVertices;
}

void OGLTiledSurface::setWarpedVertexCoords(const VertexGrid& grid)
{
    if (!m_bBound) {
        bind();
    }
    bool bGridOK = true;
    IntPoint numTiles = getNumTiles();
    if (grid.size() != (unsigned)(numTiles.y+1)) {
        bGridOK = false;
    }
    for (unsigned i = 0; i<grid.size(); ++i) {
        if (grid[i].size() != (unsigned)(numTiles.x+1)) {
            bGridOK = false;
        }
    }
    if (!bGridOK) {
        throw Exception(AVG_ERR_OUT_OF_RANGE, 
                "setWarpedVertexCoords() called with incorrect grid size.");
    }
    m_TileVertices = grid;
}

void OGLTiledSurface::bind() 
{
    if (!m_bBound) {
        calcTexCoords();
    }
    downloadTexture();
    m_bBound = true;
}

void OGLTiledSurface::blt32(const DPoint& DestSize, double opacity, 
        DisplayEngine::BlendMode Mode)
{
    glColor4d(1.0, 1.0, 1.0, opacity);
    blt(DestSize, Mode);
}

void OGLTiledSurface::blta8(const DPoint& DestSize, double opacity, 
        const Pixel32& color, DisplayEngine::BlendMode Mode)
{
    glColor4d(double(color.getR())/256, double(color.getG())/256, 
            double(color.getB())/256, opacity);
    blt(DestSize, Mode);
}

void OGLTiledSurface::blt(const DPoint& destSize, DisplayEngine::BlendMode mode)
{
    if (!m_bBound) {
        bind();
    }
    getEngine()->enableGLColorArray(false);
    getEngine()->enableTexture(true);
    getEngine()->setBlendMode(mode);
    glPushMatrix();
    glScaled(destSize.x, destSize.y, 1);

    m_pVertexes->reset();
    for (unsigned y=0; y<m_TileVertices.size()-1; y++) {
        for (unsigned x=0; x<m_TileVertices[0].size()-1; x++) {
            int curVertex=m_pVertexes->getCurVert();
            m_pVertexes->appendPos(m_TileVertices[y][x], m_TexCoords[y][x]); 
            m_pVertexes->appendPos(m_TileVertices[y][x+1], m_TexCoords[y][x+1]); 
            m_pVertexes->appendPos(m_TileVertices[y+1][x+1], m_TexCoords[y+1][x+1]); 
            m_pVertexes->appendPos(m_TileVertices[y+1][x], m_TexCoords[y+1][x]); 
            m_pVertexes->appendQuadIndexes(
                    curVertex+1, curVertex, curVertex+2, curVertex+3);
        }
    }

    activate();
    m_pVertexes->draw();
    deactivate();

    glPopMatrix();

    PixelFormat pf = getPixelFormat();
    AVG_TRACE(Logger::BLTS, "(" << destSize.x << ", " 
            << destSize.y << ")" << ", m_pf: " 
            << Bitmap::getPixelFormatString(pf) << ", " 
            << oglModeToString(getEngine()->getOGLSrcMode(pf)) << "-->" 
            << oglModeToString(getEngine()->getOGLDestMode(pf)));
}

IntPoint OGLTiledSurface::getNumTiles()
{
    IntPoint size = getSize();
    if (m_TileSize.x == -1) {
        return IntPoint(1,1);
    } else {
        return IntPoint(safeCeil(double(size.x)/m_TileSize.x),
                safeCeil(double(size.y)/m_TileSize.y));
    }
}

void OGLTiledSurface::calcVertexGrid(VertexGrid& grid)
{
    IntPoint numTiles = getNumTiles();
    std::vector<DPoint> TileVerticesLine(numTiles.x+1);
    grid = std::vector<std::vector<DPoint> > (numTiles.y+1, TileVerticesLine);
    for (unsigned y=0; y<grid.size(); y++) {
        for (unsigned x=0; x<grid[y].size(); x++) {
            calcTileVertex(x, y, grid[y][x]);
        }
    }

    if (m_pVertexes) {
        delete m_pVertexes;
    }
    m_pVertexes = new VertexArray(numTiles.x*numTiles.y*4, numTiles.x*numTiles.y*6);
}

void OGLTiledSurface::calcTileVertex(int x, int y, DPoint& Vertex) 
{
    IntPoint numTiles = getNumTiles();
    if (x < numTiles.x) {
        Vertex.x = double(m_TileSize.x*x) / getSize().x;
    } else {
        Vertex.x = 1;
    }
    if (y < numTiles.y) {
        Vertex.y = double(m_TileSize.y*y) / getSize().y;
    } else {
        Vertex.y = 1;
    }
}

void OGLTiledSurface::calcTexCoords()
{
    DPoint textureSize = DPoint(getTextureSize());
    DPoint texCoordExtents = DPoint(getSize().x/textureSize.x,
            getSize().y/textureSize.y);

    DPoint texSizePerTile;
    if (m_TileSize.x == -1) {
        texSizePerTile = texCoordExtents;
    } else {
        texSizePerTile = DPoint(double(m_TileSize.x)/getSize().x*texCoordExtents.x,
                double(m_TileSize.y)/getSize().y*texCoordExtents.y);
    }

    IntPoint numTiles = getNumTiles();
    vector<DPoint> texCoordLine(numTiles.x+1);
    m_TexCoords = std::vector<std::vector<DPoint> > 
            (numTiles.y+1, texCoordLine);
    for (unsigned y=0; y<m_TexCoords.size(); y++) {
        for (unsigned x=0; x<m_TexCoords[y].size(); x++) {
            if (y == m_TexCoords.size()-1) {
                m_TexCoords[y][x].y = texCoordExtents.y;
            } else {
                m_TexCoords[y][x].y = texSizePerTile.y*y;
            }
            if (x == m_TexCoords[y].size()-1) {
                m_TexCoords[y][x].x = texCoordExtents.x;
            } else {
                m_TexCoords[y][x].x = texSizePerTile.x*x;
            }
        }
    }
}

}
