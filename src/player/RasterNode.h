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
#include "OGLSurface.h"

#include "../avgconfigwrapper.h"
#include "../base/Point.h"

#include <string>

namespace avg {

class RasterNode: public Node
{
    public:
        static NodeDefinition getNodeDefinition();
        
        virtual ~RasterNode ();
        virtual void setDisplayEngine(DisplayEngine * pEngine);
        virtual void disconnect();
        
        // Warping support.
        VertexGrid getOrigVertexCoords();
        VertexGrid getWarpedVertexCoords();
        void setWarpedVertexCoords(const VertexGrid& Grid);

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
        OGLSurface * getOGLSurface();
        DisplayEngine::BlendMode getBlendMode() const;
        virtual std::string getTypeStr ();
        NodePtr getElementByPos (const DPoint & pos);

        Bitmap* getBitmap();
        
    protected:
        RasterNode (const ArgList& Args, Player * pPlayer);
        ISurface * getSurface();
 
    private:
        ISurface * m_pSurface;
        
        IntPoint m_MaxTileSize;
        std::string m_sBlendMode;
        DisplayEngine::BlendMode m_BlendMode;
};

}

#endif
