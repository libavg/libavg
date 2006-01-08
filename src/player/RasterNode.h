//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2006 Ulrich von Zadow
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

#include "Node.h"
#include "DisplayEngine.h"
#include "../graphics/Point.h"

#include <string>

#include "../avgconfig.h"

namespace avg {

class RasterNode: public Node
{
    public:
        virtual ~RasterNode ();
        void initVisible();
        
        // Warping support.
        int getNumVerticesX();
        int getNumVerticesY();
        DPoint getOrigVertexCoord(int x, int y);
        DPoint getWarpedVertexCoord(int x, int y);
        void setWarpedVertexCoord(int x, int y, const DPoint& Vertex);

        double getAngle() const;
        void setAngle(double Angle);
        double getPivotX() const;
        void setPivotX(double Pivotx);
        double getPivotY() const;
        void setPivotY(double Pivoty);

        int getMaxTileWidth() const
        {
            return m_MaxTileSize.x;
        }

        int getMaxTileHeight() const
        {
            return m_MaxTileSize.y;
        }
        
        const std::string& getBlendModeStr() const;
        void setBlendModeStr(const std::string& sBlendMode);
#ifdef AVG_ENABLE_GL
        OGLSurface * RasterNode::getOGLSurface();
#endif
        DisplayEngine::BlendMode getBlendMode() const;
        virtual std::string getTypeStr ();
        Node * getElementByPos (const DPoint & pos);
        
    protected:
        RasterNode ();
        RasterNode (const xmlNodePtr xmlNode, Container * pParent);
        DPoint getPivot();
        ISurface * getSurface();
 
    private:
        ISurface * m_pSurface;

        double m_Angle;
        bool m_bHasCustomPivot;
        DPoint m_Pivot;
        
        IntPoint m_MaxTileSize;
        std::string m_sBlendMode;
        DisplayEngine::BlendMode m_BlendMode;
};

}

#endif 

