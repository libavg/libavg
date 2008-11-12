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

    if (!pUnkGraph || !pdwRegister)
        return E_POINTER;

    if (FAILED(GetRunningObjectTable(0, &pROT)))
        return E_FAIL;

    hr = swprintf(wsz, NUMELMS(wsz), L"FilterGraph %08x pid %08x\0", (DWORD_PTR)pUnkGraph, 
              GetCurrentProcessId());

    hr = CreateItemMoniker(L"!", wsz, &pMoniker);
    if (SUCCEEDED(hr)) 
    {
        // Use the ROTFLAGS_REGISTRATIONKEEPSALIVE to ensure a strong reference
        // to the object.  Using this flag will cause the object to remain
        // registered until it is explicitly revoked with the Revoke() method.
        //
        // Not using this flag means that if GraphEdit remotely connects
        // to this graph and then GraphEdit exits, this object registration 
        // will be deleted, causing future attempts by GraphEdit to fail until
        // this application is restarted or until the graph is registered again.
        hr = pROT->Register(ROTFLAGS_REGISTRATIONKEEPSALIVE, pUnkGraph, 
                            pMoniker, pdwRegister);
        pMoniker->Release();
    }

    pROT->Release();
    return hr;
}

void RemoveGraphFromRot(DWORD pdwRegister)
{
    IRunningObjectTable *pROT;

    if (SUCCEEDED(GetRunningObjectTable(0, &pROT))) 
    {
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

    USES_CONVERSION;
    string s (OLE2A(varName.bstrVal));
    VariantClear(&varName);
    return s;
}

PixelFormat mediaSubtypeToPixelFormat(const GUID& Subtype, const std::string DesiredPF)
{
    if (Subtype == MEDIASUBTYPE_RGB24) {
        return B8G8R8;
    } else if (Subtype == MEDIASUBTYPE_RGB32) {
        return B8G8R8X8;
    } else if (Subtype == MEDIASUBTYPE_RGB8) {
        return I8;
    } else if (Subtype == MEDIASUBTYPE_UYVY) {
        return YCbCr422;
    } else if (Subtype == MEDIASUBTYPE_YUY2) {
        return YUYV422;   // Untested
    } else if (Subtype == MEDIASUBTYPE_RGB565) {
        return B5G6R5;
    } else if (Subtype == MEDIASUBTYPE_Y800 && DesiredPF == "BY8_GBRG") {
        return BAYER8_GBRG;
    } else if (Subtype == MEDIASUBTYPE_Y800) {
        return I8;
    } else {
        fatalError(string("mediaSubtypeToPixelFormat: Subtype ") + 
                mediaSubtypeToString(Subtype) + 
                " can't be converted to a valid pixel format.");
        return I8;
    }    
}

std::string mediaTypeToString(const GUID& Type)
{
    if (Type == MEDIATYPE_Video) {
        return "MEDIATYPE_Video";
    } else if (Type == GUID_NULL) {
        return "GUID_NULL";
    } else {
        return "Unknown";
    }
}

string mediaSubtypeToString(const GUID & Subtype)
{
    if (Subtype == MEDIASUBTYPE_RGB24) {
        return "MEDIASUBTYPE_RGB24";
    } else if (Subtype == MEDIASUBTYPE_RGB32) {
        return "MEDIASUBTYPE_RGB32";
    } else if (Subtype == MEDIASUBTYPE_RGB8) {
        return "MEDIASUBTYPE_RGB8";
    } else if (Subtype == MEDIASUBTYPE_UYVY) {
        return "MEDIASUBTYPE_UYVY";
    } else if (Subtype == MEDIASUBTYPE_YUY2) {
        return "MEDIASUBTYPE_YUY2";
    } else if (Subtype == MEDIASUBTYPE_MJPG) {
        return "MEDIASUBTYPE_MJPG";
    } else if (Subtype == MEDIASUBTYPE_RGB555) {
        return "MEDIASUBTYPE_RGB555";
    } else if (Subtype == MEDIASUBTYPE_RGB565) {
        return "MEDIASUBTYPE_RGB565";
    } else if (Subtype == MEDIASUBTYPE_Y800) {
        return "MEDIASUBTYPE_Y800";
    } else if (Subtype == GUID_NULL) {
        return "GUID_NULL";
    } else {
        return "Unknown";
    }
}

string mediaFormattypeToString(const GUID & Formattype)
{
    if (Formattype == FORMAT_None) {
        return "FORMAT_None";
    } else if (Formattype == FORMAT_VideoInfo) {
        return "FORMAT_VideoInfo";
    } else if (Formattype == FORMAT_VideoInfo2) {
        return "FORMAT_VideoInfo2";
    } else if (Formattype == GUID_NULL) {
        return "GUID_NULL";
    } else {
        return "Unknown";
    }
}

bool isDSFeatureCamControl(CameraFeature Feature)
{
    switch(Feature) {
        case CAM_FEATURE_BRIGHTNESS:
        case CAM_FEATURE_SHARPNESS:
        case CAM_FEATURE_WHITE_BALANCE:
        case CAM_FEATURE_HUE:
        case CAM_FEATURE_SATURATION:
        case CAM_FEATURE_GAMMA:
            return false;
        case CAM_FEATURE_EXPOSURE:
        case CAM_FEATURE_IRIS:
        case CAM_FEATURE_FOCUS:
        case CAM_FEATURE_ZOOM:
        case CAM_FEATURE_PAN:
        case CAM_FEATURE_TILT:
            return true;
        case CAM_FEATURE_SHUTTER:
        case CAM_FEATURE_GAIN:
        case CAM_FEATURE_TEMPERATURE:
        case CAM_FEATURE_TRIGGER:
        case CAM_FEATURE_OPTICAL_FILTER:
        case CAM_FEATURE_CAPTURE_SIZE:
        case CAM_FEATURE_CAPTURE_QUALITY:
        case CAM_FEATURE_CONTRAST:
            AVG_TRACE(Logger::WARNING, "isDSFeatureCamControl: "
                    +cameraFeatureToString(Feature)+" not supported by DirectShow.");
            return false;
        default:
            AVG_TRACE(Logger::WARNING, "isDSFeatureCamControl: "
                    +cameraFeatureToString(Feature)+" unknown.");
            return false;
    }
}

long getDSFeatureID(CameraFeature Feature)
{
    switch(Feature) {
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
        case CAM_FEATURE_GAIN:
        case CAM_FEATURE_TEMPERATURE:
        case CAM_FEATURE_TRIGGER:
        case CAM_FEATURE_OPTICAL_FILTER:
        case CAM_FEATURE_CAPTURE_SIZE:
        case CAM_FEATURE_CAPTURE_QUALITY:
        case CAM_FEATURE_CONTRAST:
            AVG_TRACE(Logger::WARNING, "getDSFeatureID: "+cameraFeatureToString(Feature)
                    +" not supported by DirectShow.");
            return 0;
        default:
            AVG_TRACE(Logger::WARNING, "getDSFeatureID: "+cameraFeatureToString(Feature)+" unknown.");
            return -1;
    }
}

}
