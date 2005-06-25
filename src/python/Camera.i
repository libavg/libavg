//
// $Id$
// 

%module avg
%{
#include "../player/Camera.h"
%}

%include "VideoBase.i"

%attribute(avg::Camera, const std::string&, device, getDevice);
%attribute(avg::Camera, double, framerate, getFrameRate);
%attribute(avg::Camera, const std::string&, mode, getMode);
%attribute(avg::Camera, int, brightness, getBrightness, setBrightness);
%attribute(avg::Camera, int, exposure, getExposure, setExposure);
%attribute(avg::Camera, int, sharpness, getSharpness, setSharpness);
%attribute(avg::Camera, int, saturation, getSaturation, setSaturation);
%attribute(avg::Camera, int, gamma, getGamma, setGamma);
%attribute(avg::Camera, int, shutter, getShutter, setShutter);
%attribute(avg::Camera, int, gain, getGain, setGain);

namespace avg {

class Camera: public VideoBase 
{
    public:
        Camera ();
        virtual ~Camera ();
};

}
