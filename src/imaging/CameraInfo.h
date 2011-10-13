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

class CamImageFormat {
public:
    CamImageFormat(IntPoint size, PixelFormat pixelFormat, std::vector<float> framerates);
    ~CamImageFormat();

    IntPoint getSize();
    PixelFormat getPixelFormat();
    FramerateList getFramerates();

private:
    IntPoint m_size;
    PixelFormat m_pixelFormat;
    FramerateList m_framerates;
};

class CamControl {
public:
    CamControl(const std::string& sControlName, int min, int max, int defaultValue);
    ~CamControl();

    std::string getControlName();
    int getMin();
    int getMax();
    int getDefault();

private:
    std::string m_sControlName;
    int m_min;
    int m_max;
    int m_defaultValue;
};

typedef std::vector<CamImageFormat> CamImageFormatsList;
typedef std::vector<CamControl> CamControlsList;

class AVG_API CameraInfo {
public:
    CameraInfo(const std::string& sDriver, const std::string& sDeviceID);
    ~CameraInfo();

    void addControl(CamControl control);
    void addImageFormat(CamImageFormat format);

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
