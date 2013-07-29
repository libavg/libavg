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

namespace avg {

CameraImageFormat::CameraImageFormat(IntPoint size, PixelFormat pixelFormat,
        FrameratesVector framerates)
{
    m_Size = size;
    m_PixelFormat = pixelFormat;
    m_Framerates = framerates;
}

CameraImageFormat::~CameraImageFormat()
{

}

IntPoint CameraImageFormat::getSize()
{
    return m_Size;
}

PixelFormat CameraImageFormat::getPixelFormat()
{
    return m_PixelFormat;
}

FrameratesVector CameraImageFormat::getFramerates()
{
    return m_Framerates;
}


CameraControl::CameraControl(const std::string& sControlName, int min, int max,
        int defaultValue)
{
    m_sControlName = sControlName;
    m_Min = min;
    m_Max = max;
    m_DefaultValue = defaultValue;
}

CameraControl::~CameraControl()
{

}

std::string CameraControl::getControlName()
{
    return m_sControlName;
}

int CameraControl::getMin()
{
    return m_Min;
}

int CameraControl::getMax()
{
    return m_Max;
}

int CameraControl::getDefault()
{
    return m_DefaultValue;
}


CameraInfo::CameraInfo(const std::string& sDriver, const std::string& sDeviceID)
{
    m_sDriver = sDriver;
    m_sDeviceID = sDeviceID;
}

CameraInfo::~CameraInfo()
{

}

void CameraInfo::addControl(CameraControl control)
{
    m_Controls.push_back(control);
}

void CameraInfo::addImageFormat(CameraImageFormat format)
{
    m_Formats.push_back(format);
}

std::string CameraInfo::getDriver()
{
    return m_sDriver;
}

std::string CameraInfo::getDeviceID()
{
    return m_sDeviceID;
}

CameraImageFormatsVector CameraInfo::getImageFormats()
{
    return m_Formats;
}

CameraControlsVector CameraInfo::getControls()
{
    return m_Controls;
}

void CameraInfo::checkAddBayer8()
{
    CameraImageFormatsVector::iterator it = m_Formats.begin();
    CameraImageFormatsVector i8ImageFormats;
    bool hasColor = false;
    for (; it!=m_Formats.end(); it++) {
        PixelFormat pf = (*it).getPixelFormat();
        if (pf == I8) {
            i8ImageFormats.push_back(*it);
        }
        if (hasColor == false) {
            hasColor = pixelFormatIsColored(pf);
        }
    }
    if (hasColor) {
        it = i8ImageFormats.begin();
        for (; it!=i8ImageFormats.end(); it++) {
                PixelFormat format = BAYER8;
                IntPoint size = (*it).getSize();
                FrameratesVector framerates = (*it).getFramerates();
                CameraImageFormat bayerImageFormat = CameraImageFormat(size, format,
                        framerates);
                m_Formats.push_back(bayerImageFormat);
        }
    }
}

}
