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

typedef std::vector<float> FrameratesVector;

class AVG_API CameraImageFormat
{
    public:
        CameraImageFormat(IntPoint size, PixelFormat pixelFormat,
                FrameratesVector framerates);
        ~CameraImageFormat();

        IntPoint getSize();
        PixelFormat getPixelFormat();
        FrameratesVector getFramerates();

    private:
        IntPoint m_Size;
        PixelFormat m_PixelFormat;
        FrameratesVector m_Framerates;
};

class AVG_API CameraControl
{
    public:
        CameraControl(const std::string& sControlName, int min, int max,
                int defaultValue);
        ~CameraControl();

        std::string getControlName();
        int getMin();
        int getMax();
        int getDefault();

    private:
        std::string m_sControlName;
        int m_Min;
        int m_Max;
        int m_DefaultValue;
};

typedef std::vector<CameraImageFormat> CameraImageFormatsVector;
typedef std::vector<CameraControl> CameraControlsVector;

class AVG_API CameraInfo
{
    public:
        CameraInfo(const std::string& sDriver, const std::string& sDeviceID);
        ~CameraInfo();

        void addControl(CameraControl control);
        void addImageFormat(CameraImageFormat format);

        std::string getDriver();
        std::string getDeviceID();
        CameraImageFormatsVector getImageFormats();
        CameraControlsVector getControls();
        void checkAddBayer8();

    private:
        std::string m_sDriver;
        std::string m_sDeviceID;
        CameraImageFormatsVector m_Formats;
        CameraControlsVector m_Controls;
};

}

#endif /* CAMERAINFO_H_ */
