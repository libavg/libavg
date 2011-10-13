//
//  libavg - Media Playback Engine.
//  Copyright (C) 2003-2011 Ulrich von Zadow
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

#ifndef CameraInfo_H_
#define CameraInfo_H_

#include "../api.h"
#include "../graphics/PixelFormat.h"
#include "../base/Point.h"

#include <string>
#include <list>
#include <map>

namespace avg{

typedef std::vector<float> FramerateList;

struct CamImageFormat {
    IntPoint size;
    PixelFormat pixelformat;
    FramerateList framerates;

    FramerateList getFramerates()
    {
        return framerates;
    }
};

struct CamControl {
    std::string sControlName;
    int min;
    int max;
    int defaultValue;
};

typedef std::vector<CamImageFormat> CamImageFormatsList;
typedef std::vector<CamControl> CamControlsList;

class AVG_API CameraInfo {
public:
    CameraInfo();
    ~CameraInfo();

    void addDriver(std::string _sDriver);
    void addDeviceID(std::string _sDeviceID);
    void addControl(CamControl _control);
    void addControl(std::string _sControlName,int _min,int _max,int _defaultValue);
    void addImageFormat(CamImageFormat _format);
    void addImageFormat(IntPoint _size, PixelFormat _pixelformat,
            FramerateList _framerates);

    std::string getDriver();
    std::string getDeviceID();
    CamImageFormatsList getImageFormats();
    CamControlsList getControls();

private:

    std::string m_sDriver;
    std::string m_sDeviceID;
    CamImageFormatsList m_formats;
    CamControlsList m_controls;

};

}

#endif /* CAMERAINFO_H_ */
