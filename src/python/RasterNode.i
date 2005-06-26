//
// $Id$
// 

%module avg
%{
#include "../player/RasterNode.h"
#include "../player/Point.h"
%}

%include "Node.i"
%include "Point.i"

%attribute(avg::RasterNode, double, angle, getAngle, setAngle);
%attribute(avg::RasterNode, double, pivotx, getPivotX, setPivotX);
%attribute(avg::RasterNode, double, pivoty, getPivotY, setPivotY);
%attribute(avg::RasterNode, int, maxtilewidth, getMaxTileWidth);
%attribute(avg::RasterNode, int, maxtileheight, getMaxTileHeight);
%attribute(avg::RasterNode, const std::string&, blendmode, getBlendModeStr, setBlendModeStr);


namespace avg {

class RasterNode: public Node
{
    public:
        virtual ~RasterNode ();
        
        int getNumVerticesX();
        int getNumVerticesY();
        avg::DPoint getOrigVertexCoord(int x, int y);
        avg::DPoint getWarpedVertexCoord(int x, int y);
        void setWarpedVertexCoord(int x, int y, const avg::DPoint& Vertex);

    protected:
        RasterNode ();
};

}

