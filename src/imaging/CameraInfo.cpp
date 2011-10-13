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
//  V4L2/libavg compliance by 02L > Outside Standing Level

#include "CameraInfo.h"

namespace avg{

    CamImageFormat::CamImageFormat(IntPoint size, PixelFormat pixelFormat, std::vector<float> framerates){
        m_size = size;
        m_pixelFormat = pixelFormat;
        m_framerates = framerates;
    }
    CamImageFormat::~CamImageFormat(){
    }

    IntPoint CamImageFormat::getSize(){
        return m_size;
    }
    PixelFormat CamImageFormat::getPixelFormat(){
        return m_pixelFormat;
    }
    FramerateList CamImageFormat::getFramerates(){
        return m_framerates;
    }


    CameraInfo::CameraInfo(const std::string& sDriver, const std::string& sDeviceID) {
        m_sDriver = sDriver;
        m_sDeviceID = sDeviceID;
    }

    CameraInfo::~CameraInfo() {
    }

    void CameraInfo::addControl(CamControl control){
        m_controls.push_back(control);
    }

    void CameraInfo::addImageFormat(CamImageFormat format){
        m_formats.push_back(format);
    }

    std::string CameraInfo::getDriver(){
        return m_sDriver;
    }

    std::string CameraInfo::getDeviceID(){
        return m_sDeviceID;
    }

    CamImageFormatsList CameraInfo::getImageFormats(){
        return m_formats;
    }

    CamControlsList CameraInfo::getControls(){
        return m_controls;
    }
}
