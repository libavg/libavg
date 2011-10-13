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
    CameraInfo::CameraInfo() {
    }

    CameraInfo::~CameraInfo() {
    }

    void CameraInfo::addDriver(std::string _sDriver){
        m_sDriver = _sDriver;
    }
    void CameraInfo::addDeviceID(std::string _sDeviceID){
        m_sDeviceID = _sDeviceID;
    }
    void CameraInfo::addControl(CamControl _control){
        m_controls.push_back(_control);
    }
    void CameraInfo::addControl(std::string _sControlName,int _min,int _max,int _defaultValue){
        CamControl control = CamControl();
        control.sControlName = _sControlName;
        control.min = _min;
        control.max = _max;
        control.defaultValue = _defaultValue;
        m_controls.push_back(control);
    }
    void CameraInfo::addImageFormat(CamImageFormat _format){
        m_formats.push_back(_format);
    }
    void CameraInfo::addImageFormat(IntPoint _size, PixelFormat _pixelformat, FramerateList _framerates){
        CamImageFormat format = CamImageFormat();
        format.size = _size;
        format.pixelformat = _pixelformat;
        format.framerates = _framerates;
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
