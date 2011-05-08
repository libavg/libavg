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

#include <InitGuid.h>

#include "DSHelper.h"
#include "DSCamera.h"
#include "DSSampleGrabber.h"

#include "../base/Logger.h"
#include "../base/Exception.h"
#include "../base/ScopeTimer.h"
#include "../graphics/FilterFlip.h"
#include "../graphics/Filtergrayscale.h"

#include <oleauto.h>

#include <math.h>

#include <sstream>

namespace avg {

using namespace std;

DSCamera::DSCamera(std::string sDevice, IntPoint size, PixelFormat camPF, 
        PixelFormat destPF, double frameRate)
    : Camera(camPF, destPF),
      m_sDevice(sDevice),
      m_Size(size),
      m_FrameRate(frameRate),
      m_pGraph(0),
      m_pCapture(0),
      m_pCameraPropControl(0)
{
    open();
}

DSCamera::~DSCamera()
{
    close();
}

void DSCamera::open()
{
    initGraphBuilder();
    findCaptureDevice(&m_pSrcFilter);
    if (m_pSrcFilter) {
        HRESULT hr;
        hr = m_pGraph->AddFilter(m_pSrcFilter, L"Video Capture");
        checkForDShowError(hr, "DSCamera::open()::Add capture filter");

        // Create and configure the sample grabber that delivers the frames to the app.
        m_pGrabFilter = new CSampleGrabber(NULL, &hr);
        m_pGrabFilter->AddRef();
        checkForDShowError(hr, "DSCamera::open()::Create SampleGrabber");
        hr = m_pGrabFilter->QueryInterface(IID_IlibavgGrabber, 
                (void **)&m_pSampleGrabber);
        checkForDShowError(hr, "DSCamera::open()::Create SampleGrabber 2");

        hr = m_pSrcFilter->QueryInterface(IID_IAMVideoProcAmp, 
                (void **)&m_pCameraPropControl);
        checkForDShowError(hr, "DSCamera::open()::get IAMVideoProcAmp");
        hr = m_pSrcFilter->QueryInterface(IID_IAMCameraControl, 
                (void **)&m_pAMCameraControl);
        checkForDShowError(hr, "DSCamera::open()::get IAMCameraControl");

        hr = m_pGraph->AddFilter(m_pGrabFilter, L"Sample Grabber");
        checkForDShowError(hr, "DSCamera::open()::Add Grabber");
        setCaptureFormat();

        checkForDShowError(hr, "DSCamera::open()::SetMediaType");

        m_pSampleGrabber->SetCallback(this);

        IBaseFilter * pNull;
        hr = CoCreateInstance(CLSID_NullRenderer, NULL, CLSCTX_INPROC_SERVER, 
                IID_IBaseFilter, (LPVOID*) &pNull);
        checkForDShowError(hr, "DSCamera::open()::Create null filter");
        m_pGraph->AddFilter(pNull, L"NullRender");
        pNull->Release();

        connectFilters(m_pGraph, m_pSrcFilter, m_pGrabFilter);
        connectFilters(m_pGraph, m_pGrabFilter, pNull);

        // Add our graph to the running object table, which will allow
        // the GraphEdit application to "spy" on our graph.
        hr = AddGraphToRot(m_pGraph, &m_GraphRegisterID);
        checkForDShowError(hr, "DSCamera::open()::AddGraphToRot");

    } else {
        throw (Exception(AVG_ERR_CAMERA_NONFATAL, "DS Camera unavailable"));
    }
}

void DSCamera::startCapture()
{
    HRESULT hr = m_pMediaControl->Run();
    checkForDShowError(hr, "DSCamera::open()::Run");
}

void DSCamera::close()
{
    if (m_pAMCameraControl) {
        m_pAMCameraControl->Release();
    }
    m_pMediaControl->Stop();
    RemoveGraphFromRot(m_GraphRegisterID);
    m_pGraph->Release();
    m_pCapture->Release();
    m_pMediaControl->Release();
    if (m_pCameraPropControl) {
        m_pCameraPropControl->Release();
    }
    m_pSrcFilter->Release();
    m_pGrabFilter->Release();
    m_pSampleGrabber->Release();
}

IntPoint DSCamera::getImgSize()
{
    return m_Size;
}

BitmapPtr DSCamera::getImage(bool bWait)
{
    BitmapPtr pBmp;
    try {
        pBmp = m_BitmapQ.pop(bWait);
    } catch (Exception&) {
        return BitmapPtr();
    }

    return pBmp;
}

void DSCamera::setCaptureFormat()
{
    IAMStreamConfig *pSC;
    HRESULT hr = m_pCapture->FindInterface(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Video, 
            m_pSrcFilter, IID_IAMStreamConfig, (void **)&pSC);
    checkForDShowError(hr, "DSCamera::setCaptureFormat::FindInterface");

    int numCaps = 0;
    int capsSize = 0;
    hr = pSC->GetNumberOfCapabilities(&numCaps, &capsSize);
    checkForDShowError(hr, "DSCamera::dumpMediaTypes::GetNumberOfCapabilities");

    AVG_ASSERT(capsSize == sizeof(VIDEO_STREAM_CONFIG_CAPS));
    bool bFormatFound = false;
    bool bCloseFormatFound = false;
    AM_MEDIA_TYPE* pmtConfig;
    AM_MEDIA_TYPE* pmtCloseConfig;
    vector<string> sImageFormats;
    VIDEOINFOHEADER* pvih;
    BITMAPINFOHEADER bih;
    PixelFormat capsPF;
    for (int i = 0; i < numCaps; i++) {
        VIDEO_STREAM_CONFIG_CAPS scc;
        hr = pSC->GetStreamCaps(i, &pmtConfig, (BYTE*)&scc);
        checkForDShowError(hr, "DSCamera::dumpMediaTypes::GetStreamCaps");
        pvih = (VIDEOINFOHEADER*)(pmtConfig->pbFormat);
        bih = pvih->bmiHeader;
        double frameRate = double(10000000L/pvih->AvgTimePerFrame);
        capsPF = mediaSubtypeToPixelFormat(pmtConfig->subtype);

        if (capsPF != NO_PIXELFORMAT && bih.biWidth != 0) {
            sImageFormats.push_back(camImageFormatToString(pmtConfig));
        }

        bool bFormatUsed = false;
        int height = bih.biHeight;
        if (height < 0) {
            height = -height;
        }
        if (bih.biWidth == m_Size.x && height == m_Size.y && 
                (getCamPF() == capsPF || (getCamPF() == BAYER8_GBRG && capsPF == I8)))
        {
            if (fabs(m_FrameRate-frameRate) < 0.001) {
                bFormatFound = true;
                break;
            } else if (!bCloseFormatFound) {
                // The current format fits everything but the framerate.
                // Not all framerates are reported, so we're going to try this one as 
                // well.
                bCloseFormatFound = true;
                bFormatUsed = true;
                pmtCloseConfig = pmtConfig;
            }
        }
        if (!bFormatUsed) {
            CoTaskMemFree((PVOID)pmtConfig->pbFormat);
            CoTaskMemFree(pmtConfig);
        }
    }
    if (bFormatFound) {
        AVG_TRACE(Logger::CONFIG, "Camera image format: "
                << camImageFormatToString(pmtConfig));
        int height = ((VIDEOINFOHEADER*)(pmtConfig->pbFormat))->bmiHeader.biHeight;
//        m_bUpsideDown = (height < 0);
        hr = pSC->SetFormat(pmtConfig);
        checkForDShowError(hr, "DSCamera::dumpMediaTypes::SetFormat");
        CoTaskMemFree((PVOID)pmtConfig->pbFormat);
        CoTaskMemFree(pmtConfig);
    } else {
        if (bCloseFormatFound) {
            // Set the framerate manually.
            pvih = (VIDEOINFOHEADER*)(pmtCloseConfig->pbFormat);
            pvih->AvgTimePerFrame = REFERENCE_TIME(10000000/m_FrameRate);
            int height = pvih->bmiHeader.biHeight;
//            m_bUpsideDown = (height < 0);
            hr = pSC->SetFormat(pmtCloseConfig);
            checkForDShowError(hr, "DSCamera::dumpMediaTypes::SetFormat");
            AVG_TRACE(Logger::CONFIG, "Camera image format: " 
                    << camImageFormatToString(pmtCloseConfig));
            CoTaskMemFree((PVOID)pmtCloseConfig->pbFormat);
            CoTaskMemFree(pmtCloseConfig);

            // TODO: Check if framerate is actually attained.
        } else {
            AVG_TRACE(Logger::WARNING, 
                "Possibly incomplete list of camera image formats: ");
            for (unsigned i = 0; i < sImageFormats.size(); i++) {
                AVG_TRACE(Logger::WARNING, "  " << sImageFormats[i]);
            }
            throw Exception(AVG_ERR_CAMERA_NONFATAL, 
                    "Could not find requested camera image format.");
        }
    }
    pSC->Release();
}


const string& DSCamera::getDevice() const
{
    return m_sDevice;
}

const std::string& DSCamera::getDriverName() const
{
    static string sDriverName = "directshow";
    return sDriverName;
}

double DSCamera::getFrameRate() const
{
    return m_FrameRate;
}

int DSCamera::getFeature(CameraFeature feature) const
{
    long prop = getDSFeatureID(feature);
    long val;
    long flags;
    HRESULT hr;
    if (isDSFeatureCamControl(feature)) {
        hr = m_pAMCameraControl->Get(prop, &val, &flags);
    } else {
        hr = m_pCameraPropControl->Get(prop, &val, &flags);
    }
    if (!SUCCEEDED(hr)) {
        AVG_TRACE(Logger::WARNING, "DSCamera::getFeature "
                + cameraFeatureToString(feature)+" failed.");
        return 0;
    }
    return val;
}

void DSCamera::setFeature(CameraFeature feature, int value, bool bIgnoreOldValue)
{
    long prop = getDSFeatureID(feature);
    if (!m_pCameraPropControl) {
        return;
    }
    long flags;
    if (value == -1) {
        flags = VideoProcAmp_Flags_Auto;
    } else {
        flags = VideoProcAmp_Flags_Manual;
    }
    HRESULT hr;
    if (isDSFeatureCamControl(feature)) {
        hr = m_pAMCameraControl->Set(prop, value, flags);
    } else {
        hr = m_pCameraPropControl->Set(prop, value, flags);
    }
    switch (hr) {
        case E_INVALIDARG:
            // TODO: Throw exception
            AVG_TRACE(Logger::ERROR, "DSCamera::setFeature(" 
                    << cameraFeatureToString(feature) << ", " << value << ") failed.");
            break;
        case E_PROP_ID_UNSUPPORTED:  
        case E_PROP_SET_UNSUPPORTED:
            AVG_TRACE(Logger::ERROR, "DSCamera::setFeature(" 
                << cameraFeatureToString(feature)
                << ") failed: Feature not supported by camera.");
            break;
        default:
            checkForDShowError(hr, "DSCamera::setFeature()::Set value");
    }
}

void DSCamera::setFeatureOneShot(CameraFeature feature)
{
    AVG_TRACE(Logger::WARNING, 
            "OneShot feature setting not implemented for DirectShow camera driver.");
}

int DSCamera::getWhitebalanceU() const
{
    AVG_TRACE(Logger::WARNING, 
            "Whitebalance not implemented for DirectShow camera driver.");
    return 0;
}

int DSCamera::getWhitebalanceV() const
{
    AVG_TRACE(Logger::WARNING, 
            "Whitebalance not implemented for DirectShow camera driver.");
    return 0;
}

void DSCamera::setWhitebalance(int u, int v, bool bIgnoreOldValue)
{
    AVG_TRACE(Logger::WARNING, 
            "Whitebalance not implemented for DirectShow camera driver.");
}

void DSCamera::onSample(IMediaSample * pSample)
{
    unsigned char * pData;

    // Get the current image.
    pSample->GetPointer(&pData);

    int stride = m_Size.x*Bitmap::getBytesPerPixel(getCamPF());
    Bitmap camBmp(m_Size, getCamPF(), pData, stride, false, "CameraImage");
    // Copy over to bitmap queue, doing pixel format conversion if necessary.
    BitmapPtr pDestBmp = BitmapPtr(new Bitmap(m_Size, getDestPF(), 
            "ConvertedCameraImage"));
    pDestBmp->copyPixels(camBmp);
/*
    if (m_bUpsideDown) {
        FilterFlip().applyInPlace(pDestBmp);
    }
*/
    m_BitmapQ.push(pDestBmp);
}

void DSCamera::dumpCameras()
{
    HRESULT hr = S_OK;
    // TODO: Check if the threading model is ok.
    hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    checkForDShowError(hr, "DSCamera::dumpCameras()::CoInitializeEx");

    // Create the system device enumerator
    ICreateDevEnum *pDevEnum =NULL;
    hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC, 
            IID_ICreateDevEnum, (void **) &pDevEnum);
    checkForDShowError(hr, "DSCamera::dumpCameras()::CreateDevEnum");

    // Create an enumerator for the video capture devices
    IEnumMoniker *pClassEnum = NULL;
    hr = pDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pClassEnum, 0);
    checkForDShowError(hr, "DSCamera::dumpCameras()::CreateClassEnumerator");

    if (pClassEnum == NULL) {
        return;
    }

    IMoniker* pMoniker = NULL;
    bool bFirst = true;
    while (pClassEnum->Next(1, &pMoniker, NULL) == S_OK) {
        if (bFirst) {
            cerr << endl;
            cerr << "DirectShow cameras: " << endl;
            bFirst = false;
        }
        IPropertyBag* pPropBag;
        hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag, (void**)(&pPropBag));
        checkForDShowError(hr, "DSCamera::dumpCameras()::BindToStorage");
        cerr << "  ----------------------------" << endl;
        cerr << "  Name: " << getStringProp(pPropBag, L"FriendlyName") << endl;
        cerr << "  Description: " << getStringProp(pPropBag, L"Description") << endl;
        cerr << "  Device Path: " << getStringProp(pPropBag, L"DevicePath") << endl;
        pPropBag->Release();
    }
    pMoniker->Release();
    pDevEnum->Release();
    pClassEnum->Release();
}

void DSCamera::initGraphBuilder()
{
    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    checkForDShowError(hr, "DSCamera::initGraphBuilder()::CoInitializeEx");

    // Create the filter graph
    hr = CoCreateInstance (CLSID_FilterGraph, NULL, CLSCTX_INPROC, IID_IGraphBuilder, 
            (void **) &m_pGraph);
    checkForDShowError(hr, "DSCamera::initGraphBuilder()::GraphBuilder");
    // Create the capture graph builder
    hr = CoCreateInstance (CLSID_CaptureGraphBuilder2 , NULL, CLSCTX_INPROC,
            IID_ICaptureGraphBuilder2, (void **) &m_pCapture);   
    checkForDShowError(hr, "DSCamera::initGraphBuilder()::CaptureGraphBuilder2");

    hr = m_pCapture->SetFiltergraph(m_pGraph);
    checkForDShowError(hr, "DSCamera::initGraphBuilder()::SetFilterGraph");

    m_pGraph->QueryInterface(IID_IMediaControl,(void**)&m_pMediaControl);
}

void DSCamera::findCaptureDevice(IBaseFilter ** ppSrcFilter)
{
    HRESULT hr = S_OK;
    IBaseFilter * pSrc = NULL;
    IMoniker* pMoniker = NULL;
    ICreateDevEnum *pDevEnum = NULL;
    IEnumMoniker *pClassEnum = NULL;

    // Create the system device enumerator
    hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC, 
            IID_ICreateDevEnum, (void **) &pDevEnum);
    checkForDShowError(hr, "DSCamera::findCaptureDevice()::CreateDevEnum");

    // Create an enumerator for the video capture devices
    hr = pDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pClassEnum, 
            0);
    checkForDShowError(hr, "DSCamera::findCaptureDevice()::CreateClassEnumerator");

    // If there are no enumerators for the requested type, then 
    // CreateClassEnumerator will succeed, but pClassEnum will be NULL.
    if (pClassEnum == NULL) {
        *ppSrcFilter = 0;
        throw(Exception(AVG_ERR_CAMERA_NONFATAL, "No DirectShow Capture Device found"));
    }

    bool bFound = false;
    while (!bFound && pClassEnum->Next(1, &pMoniker, NULL) == S_OK) {
        IPropertyBag *pPropBag;
        hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag, (void**)(&pPropBag));
        checkForDShowError(hr, "DSCamera::findCaptureDevice()::BindToStorage");

        string sDescription = getStringProp(pPropBag, L"Description");
        string sFriendlyName = getStringProp(pPropBag, L"FriendlyName");
        string sDevicePath = getStringProp(pPropBag, L"DevicePath");

        if (m_sDevice == sDescription  || m_sDevice == sFriendlyName || 
                sDevicePath.find(m_sDevice) != -1 || m_sDevice == "")
        {
            bFound = true;
        } else {
            pMoniker->Release();
        }
        pPropBag->Release();
    }
    if (!bFound) {
        pClassEnum->Reset();
        if (pClassEnum->Next(1, &pMoniker, NULL) == S_OK) {
            AVG_TRACE(Logger::WARNING, string("Camera ") + m_sDevice
                    + " not found. Using first camera.");
            bFound = true;
            IPropertyBag *pPropBag;
            hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag, (void**)(&pPropBag));
            m_sDevice = getStringProp(pPropBag, L"FriendlyName");
            checkForDShowError(hr, "DSCamera::findCaptureDevice()::BindToStorage");
            pPropBag->Release();
        } else {
            throw(Exception(AVG_ERR_CAMERA_NONFATAL, 
                    "No DirectShow Capture Device found"));
        }
    }

    // Bind Moniker to a filter object
    hr = pMoniker->BindToObject(0,0,IID_IBaseFilter, (void**)&pSrc);
    checkForDShowError(hr, "DSCamera::findCaptureDevice()::BindToObject");

    // Copy the found filter pointer to the output parameter.
    *ppSrcFilter = pSrc;
    (*ppSrcFilter)->AddRef();

    pSrc->Release();
    pMoniker->Release();
    pDevEnum->Release();
    pClassEnum->Release();
}


void DSCamera::connectFilters(IGraphBuilder *pGraph, IBaseFilter *pSrc, 
        IBaseFilter *pDest)
{
    IPin *pOut = 0;
    getUnconnectedPin(pSrc, PINDIR_OUTPUT, &pOut);
    
    IPin *pIn = 0;
    getUnconnectedPin(pDest, PINDIR_INPUT, &pIn);
    
    HRESULT hr = pGraph->ConnectDirect(pOut, pIn, 0);
    checkForDShowError(hr, "DSCamera::ConnectFilters::Connect");
    pOut->Release();
    pIn->Release();
}

void DSCamera::getUnconnectedPin(IBaseFilter *pFilter, PIN_DIRECTION pinDir, IPin **ppPin)
{
    *ppPin = 0;
    IEnumPins *pEnum = 0;
    IPin *pPin = 0;
    HRESULT hr = pFilter->EnumPins(&pEnum);
    checkForDShowError(hr, "DSCamera::ConnectFilters::Connect");
    while (pEnum->Next(1, &pPin, NULL) == S_OK)
    {
        PIN_DIRECTION thisPinDir;
        pPin->QueryDirection(&thisPinDir);
        if (thisPinDir == pinDir) {
            IPin* pTmp = 0;
            hr = pPin->ConnectedTo(&pTmp);
            if (SUCCEEDED(hr)) { // Already connected, not the pin we want.
                pTmp->Release();
            } else { // Unconnected, this is the pin we want.
                pEnum->Release();
                *ppPin = pPin;
                return;
            }
        }
        pPin->Release();
    }
    pEnum->Release();
    // Did not find a matching pin.
    AVG_ASSERT(false);
}

#pragma warning(disable : 4995)
void DSCamera::checkForDShowError(HRESULT hr, const string& sLocation)
{
    if (SUCCEEDED(hr)) {
        return;
    }
    if (HRESULT_FACILITY(hr) == FACILITY_WINDOWS) {
        hr = HRESULT_CODE(hr);
    }
    char szErr[MAX_ERROR_TEXT_LEN];
    DWORD res = AMGetErrorText(hr, szErr, MAX_ERROR_TEXT_LEN);
    if (res == 0) {
        wsprintf(szErr, "Unknown Error: 0x%2x", hr);
    }
    cerr << sLocation << ": " << szErr << endl;
}


}
