//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2008 Ulrich von Zadow
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

#include "DSHelper.h"

#include "../base/Exception.h"
#include "../base/Logger.h"

#include <stdio.h>
#include <oleauto.h>

#include <string>

using namespace std;

namespace avg {

#pragma warning(disable : 4995)
HRESULT AddGraphToRot(IUnknown *pUnkGraph, DWORD *pdwRegister) 
{
    IMoniker * pMoniker;
    IRunningObjectTable *pROT;
    WCHAR wsz[128];
    HRESULT hr;

    if (!pUnkGraph || !pdwRegister) {
        return E_POINTER;
    }

    if (FAILED(GetRunningObjectTable(0, &pROT))) {
        return E_FAIL;
    }

    hr = swprintf(wsz, NUMELMS(wsz), L"FilterGraph %08x pid %08x\0",
            (DWORD_PTR)pUnkGraph, GetCurrentProcessId());

    hr = CreateItemMoniker(L"!", wsz, &pMoniker);
    if (SUCCEEDED(hr)) {
        hr = pROT->Register(ROTFLAGS_REGISTRATIONKEEPSALIVE, pUnkGraph, pMoniker, 
                pdwRegister);
        pMoniker->Release();
    }

    pROT->Release();
    return hr;
}

void RemoveGraphFromRot(DWORD pdwRegister)
{
    IRunningObjectTable *pROT;

    if (SUCCEEDED(GetRunningObjectTable(0, &pROT))) {
        pROT->Revoke(pdwRegister);
        pROT->Release();
    }
}

string getStringProp(IPropertyBag *pPropBag, LPCOLESTR pszPropName)
{
    VARIANT varName;
    VariantInit(&varName);
    HRESULT hr = pPropBag->Read(pszPropName, &varName, 0);
    if (FAILED(hr)) {
        return "";
    }
//    checkForDShowError(hr, string("DSHelper::getStringProp(")+pszPropName+")::Read PropBag");

    string s;
    int lenWStr = SysStringLen(varName.bstrVal);
    int lenAStr = WideCharToMultiByte(CP_ACP, 0, varName.bstrVal, lenWStr, 0, 0, NULL, NULL);
    if (lenAStr > 0) {
        char* pAStr = new char[lenAStr + 1];
        WideCharToMultiByte(CP_ACP, 0, varName.bstrVal, lenWStr, pAStr, lenAStr, NULL, NULL);
        pAStr[lenAStr] = 0;
        s = pAStr;
        delete[] pAStr;
    }

    VariantClear(&varName);
    return s;
}

PixelFormat mediaSubtypeToPixelFormat(const GUID& subtype)
{
    if (subtype == MEDIASUBTYPE_RGB24) {
        return B8G8R8;
    } else if (subtype == MEDIASUBTYPE_RGB32) {
        return B8G8R8X8;
    } else if (subtype == MEDIASUBTYPE_RGB8) {
        return I8;
    } else if (subtype == MEDIASUBTYPE_UYVY) {
        return YCbCr422;
    } else if (subtype == MEDIASUBTYPE_YUY2) {
        return YUYV422;
    } else if (subtype == MEDIASUBTYPE_RGB565) {
        return B5G6R5;
//    } else if (Subtype == MEDIASUBTYPE_Y800 && DesiredPF == "BY8_GBRG") {
//        return BAYER8_GBRG;
    } else if (subtype == MEDIASUBTYPE_Y800) {
        return I8;
    } else {
        return NO_PIXELFORMAT;
    }    
}

std::string mediaTypeToString(const GUID& type)
{
    if (type == MEDIATYPE_Video) {
        return "MEDIATYPE_Video";
    } else if (type == GUID_NULL) {
        return "GUID_NULL";
    } else {
        return "Unknown";
    }
}

string mediaSubtypeToString(const GUID & subtype)
{
    if (subtype == MEDIASUBTYPE_RGB24) {
        return "MEDIASUBTYPE_RGB24";
    } else if (subtype == MEDIASUBTYPE_RGB32) {
        return "MEDIASUBTYPE_RGB32";
    } else if (subtype == MEDIASUBTYPE_RGB8) {
        return "MEDIASUBTYPE_RGB8";
    } else if (subtype == MEDIASUBTYPE_UYVY) {
        return "MEDIASUBTYPE_UYVY";
    } else if (subtype == MEDIASUBTYPE_YUY2) {
        return "MEDIASUBTYPE_YUY2";
    } else if (subtype == MEDIASUBTYPE_MJPG) {
        return "MEDIASUBTYPE_MJPG";
    } else if (subtype == MEDIASUBTYPE_RGB555) {
        return "MEDIASUBTYPE_RGB555";
    } else if (subtype == MEDIASUBTYPE_RGB565) {
        return "MEDIASUBTYPE_RGB565";
    } else if (subtype == MEDIASUBTYPE_Y800) {
        return "MEDIASUBTYPE_Y800";
    } else if (subtype == GUID_NULL) {
        return "GUID_NULL";
    } else {
        return "Unknown";
    }
}

string mediaFormattypeToString(const GUID& formattype)
{
    if (formattype == FORMAT_None) {
        return "FORMAT_None";
    } else if (formattype == FORMAT_VideoInfo) {
        return "FORMAT_VideoInfo";
    } else if (formattype == FORMAT_VideoInfo2) {
        return "FORMAT_VideoInfo2";
    } else if (formattype == GUID_NULL) {
        return "GUID_NULL";
    } else {
        return "Unknown";
    }
}

string camImageFormatToString(const AM_MEDIA_TYPE* pMediaType)
{
    stringstream ss;
    VIDEOINFOHEADER* pVideoInfo = (VIDEOINFOHEADER*)(pMediaType->pbFormat);
    BITMAPINFOHEADER* pBitmapInfo = &pVideoInfo->bmiHeader;
    PixelFormat pf = mediaSubtypeToPixelFormat(pMediaType->subtype);
    ss << "(" << pBitmapInfo->biWidth << "x" << pBitmapInfo->biHeight << "), " << pf 
            << ", " << 10000000./pVideoInfo->AvgTimePerFrame << " fps.";
    return ss.str();
}

bool isDSFeatureCamControl(CameraFeature feature)
{
    switch(feature) {
        case CAM_FEATURE_BRIGHTNESS:
        case CAM_FEATURE_SHARPNESS:
        case CAM_FEATURE_WHITE_BALANCE:
        case CAM_FEATURE_HUE:
        case CAM_FEATURE_SATURATION:
        case CAM_FEATURE_GAMMA:
        case CAM_FEATURE_GAIN:
            return false;
        case CAM_FEATURE_EXPOSURE:
        case CAM_FEATURE_IRIS:
        case CAM_FEATURE_FOCUS:
        case CAM_FEATURE_ZOOM:
        case CAM_FEATURE_PAN:
        case CAM_FEATURE_TILT:
            return true;
        case CAM_FEATURE_SHUTTER:
        case CAM_FEATURE_TEMPERATURE:
        case CAM_FEATURE_TRIGGER:
        case CAM_FEATURE_OPTICAL_FILTER:
        case CAM_FEATURE_CAPTURE_SIZE:
        case CAM_FEATURE_CAPTURE_QUALITY:
        case CAM_FEATURE_CONTRAST:
            AVG_TRACE(Logger::WARNING, "isDSFeatureCamControl: "
                    + cameraFeatureToString(feature) + " not supported by DirectShow.");
            return false;
        default:
            AVG_TRACE(Logger::WARNING, "isDSFeatureCamControl: "
                    + cameraFeatureToString(feature) + " unknown.");
            return false;
    }
}

long getDSFeatureID(CameraFeature feature)
{
    switch(feature) {
        case CAM_FEATURE_BRIGHTNESS:
            return VideoProcAmp_Brightness;
        case CAM_FEATURE_SHARPNESS:
            return VideoProcAmp_Sharpness;
        case CAM_FEATURE_WHITE_BALANCE:
            return VideoProcAmp_WhiteBalance;
        case CAM_FEATURE_HUE:
            return VideoProcAmp_Hue;
        case CAM_FEATURE_SATURATION:
            return VideoProcAmp_Saturation;
        case CAM_FEATURE_GAMMA:
            return VideoProcAmp_Gamma;
        case CAM_FEATURE_GAIN:
            return VideoProcAmp_Gain;
        case CAM_FEATURE_EXPOSURE:
            return CameraControl_Exposure;
        case CAM_FEATURE_IRIS:
            return CameraControl_Iris;
        case CAM_FEATURE_FOCUS:
            return CameraControl_Focus;
        case CAM_FEATURE_ZOOM:
            return CameraControl_Zoom;
        case CAM_FEATURE_PAN:
            return CameraControl_Pan;
        case CAM_FEATURE_TILT:
            return CameraControl_Tilt;
        case CAM_FEATURE_SHUTTER:
        case CAM_FEATURE_TEMPERATURE:
        case CAM_FEATURE_TRIGGER:
        case CAM_FEATURE_OPTICAL_FILTER:
        case CAM_FEATURE_CAPTURE_SIZE:
        case CAM_FEATURE_CAPTURE_QUALITY:
        case CAM_FEATURE_CONTRAST:
            AVG_TRACE(Logger::WARNING, "getDSFeatureID: "+cameraFeatureToString(feature)
                    +" not supported by DirectShow.");
            return 0;
        default:
            AVG_TRACE(Logger::WARNING, "getDSFeatureID: "+cameraFeatureToString(feature)+" unknown.");
            return -1;
    }
}

}
