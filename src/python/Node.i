//
// $Id$
// 

%module avg
%{
#include "../player/Node.h"
%}

%attribute(avg::Node, const std::string&, ID, getID);
%attribute(avg::Node, double, x, getX, setX);
%attribute(avg::Node, double, y, getY, setY);
%attribute(avg::Node, int, z, getZ, setZ);
%attribute(avg::Node, double, width, getWidth, setWidth);
%attribute(avg::Node, double, height, getHeight, setHeight);
%attribute(avg::Node, double, opacity, getOpacity, setOpacity);
%attribute(avg::Node, double, active, getActive, setActive);

namespace avg {

class Node
{
    public:
        /**
         * Returns the parent node, if there is one.
         */
//        virtual Container * getParent();
    protected:
        Node();
};

}

