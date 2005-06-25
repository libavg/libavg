//
// $Id$
// 

%module avg
%{
#include "../player/PanoImage.h"
%}

%include "Node.i"

%attribute(avg::PanoImage, const std::string&, href, getFilename);
%attribute(avg::PanoImage, double, SensorWidth, getSensorWidth);
%attribute(avg::PanoImage, double, SensorHeight, getSensorHeight);
%attribute(avg::PanoImage, double, FocalLength, getFocalLength, setFocalLength);
%attribute(avg::PanoImage, int, Hue, getHue);
%attribute(avg::PanoImage, int, Saturation, getSaturation);
%attribute(avg::PanoImage, double, Rotation, getRotation, setRotation);
%attribute(avg::PanoImage, double, MaxRotation, getMaxRotation);

namespace avg {

class PanoImage : public Node
{
	public:
        PanoImage ();
        virtual ~PanoImage ();
};

}

