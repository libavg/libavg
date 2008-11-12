// Copyright (C) 2008 Archimedes Solutions GmbH,
// Saarbrücker Str. 24b, Berlin, Germany
//
// This file contains proprietary source code and confidential
// information. Its contents may not be disclosed or distributed to
// third parties unless prior specific permission by Archimedes
// Solutions GmbH, Berlin, Germany is obtained in writing. This applies
// to copies made in any form and using any medium. It applies to
// partial as well as complete copies.

#include "DSHelper.h"
#include "DSCamera.h"
#include "Ks.h"
#include "KsMedia.h"

#include "../base/Logger.h"
#include "../base/ScopeTimer.h"
#include "../base/TimeSource.h"
#include "../graphics/FilterFlip.h"
#include "../graphics/Filtergrayscale.h"

#include <assert.h>
#include <oleauto.h>
//#include <atlcomcli.h>

#include <math.h>

#include <sstream>

namespace avg {

using namespace std;

DSCamera::DSCamera(std::string sDevice, IntPoint Size, std::string sPF,
            double FrameRate, bool bColor)
    : m_sDevice(sDevice),
      m_Size(Size),
      m_FrameRate(FrameRate),
      m_bColor(bColor),
      m_bCameraAvailable(false),
      m_pGraph(0),
      m_pCapture(0),
      m_pCameraPropControl(0),
      m_pSampleQueue(0),
      m_sPF(sPF)
{
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

        // Create and configure the sample grabber that delivers the frames to
        // the app.
	    hr = CoCreateInstance(CLSID_SampleGrabber, NULL, 
                CLSCTX_INPROC_SERVER, IID_IBaseFilter, (LPVOID *)&m_pGrabFilter);
        checkForDShowError(hr, "DSCamera::open()::Create SampleGrabber");
	    hr = m_pGrabFilter->QueryInterface(IID_ISampleGrabber, 
                (void **)&m_pSampleGrabber);
        checkForDShowError(hr, "DSCamera::open()::Create SampleGrabber 2");

        hr = m_pSrcFilter->QueryInterface(IID_IAMVideoProcAmp, 
                (void **)&m_pCameraPropControl);
        checkForDShowError(hr, "DSCamera::getImage()::get IAMVideoProcAmp");
        hr = m_pSrcFilter->QueryInterface(IID_IAMCameraControl, 
                (void **)&m_pAMCameraControl);
        checkForDShowError(hr, "DSCamera::getImage()::get IAMCameraControl");


        hr = m_pGraph->AddFilter(m_pGrabFilter, L"Sample Grabber");
        checkForDShowError(hr, "DSCamera::open()::Add Grabber");
        setCaptureFormat();

        AM_MEDIA_TYPE mt;
        ZeroMemory(&mt, sizeof(AM_MEDIA_TYPE));
        mt.majortype = MEDIATYPE_Video;
        if (!m_bCameraIsColor) {
            mt.subtype = MEDIASUBTYPE_Y800;
        }

        hr = m_pSampleGrabber->SetMediaType(&mt);
        checkForDShowError(hr, "DSCamera::open()::SetMediaType");

        PixelFormat DestPF;
        if (m_sPF == "BY8" || m_sPF == "YUV422") {
            DestPF = B8G8R8X8;
        } else if (m_sPF == "MONO8") {
            DestPF = I8;
        } else if (m_bColor) {
            DestPF = B8G8R8X8;
        } else {
            DestPF = I8;
        }

        m_pSampleQueue = new DSSampleQueue(m_Size, m_CameraPF, DestPF);
	    m_pSampleGrabber->SetCallback(m_pSampleQueue, 0);

/*
        cerr << "Grabber type: " << mediaTypeToString(mt.majortype) << 
                ", subtype: " << mediaSubtypeToString(mt.subtype) << 
                ", formattype: " << mediaFormattypeToString(mt.formattype) <<
                endl;
*/

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

        hr = m_pMediaControl->Run(); // Start capturing
        checkForDShowError(hr, "DSCamera::open()::Run");

        m_bCameraAvailable = true;
    } else {
        m_bCameraAvailable = false;
    }
}

void DSCamera::close()
{
    if (m_bCameraAvailable) {
        m_pAMCameraControl->Release();
        m_pMediaControl->Stop();
        RemoveGraphFromRot(m_GraphRegisterID);
        m_pGraph->Release();
        m_pCapture->Release();
        m_pMediaControl->Release();
        m_pCameraPropControl->Release();
        m_pSrcFilter->Release();
        m_pGrabFilter->Release();
        m_pSampleGrabber->Release();
        delete m_pSampleQueue;
    }
    m_bCameraAvailable = false;
}

IntPoint DSCamera::getImgSize()
{
    return m_Size;
}

static ProfilingZone CameraConvertProfilingZone("DS Camera format conversion");

BitmapPtr DSCamera::getImage(bool bWait)
{
    if (!m_bCameraAvailable && bWait) {
        msleep(1000);
        open();
    }
    if (!m_bCameraAvailable) {
        // Open failed
        return BitmapPtr();
    }

    bool bGotFrame = true;
    BitmapPtr pBmp;
    try {
        pBmp = m_pSampleQueue->getImage(bWait);
    } catch (Exception&) {
        return BitmapPtr();
    }

    return pBmp;
}

void DSCamera::setCaptureFormat()
{

    // First, look for a camera image format that matches the color selection
    bool bFound = selectMediaType(m_bColor, false);
    // Then, look for one that doesn't match and convert the images afterwards.
    if (!bFound) {
        selectMediaType(!m_bColor, true);
    }
}

bool DSCamera::selectMediaType(bool bColor, bool bForce) 
{
	IAMStreamConfig *pSC;
    HRESULT hr = m_pCapture->FindInterface(&PIN_CATEGORY_CAPTURE,
			&MEDIATYPE_Video, m_pSrcFilter, IID_IAMStreamConfig, (void **)&pSC);
    checkForDShowError(hr, "DSCamera::setCaptureFormat::FindInterface");

    int Count = 0;
    int Size = 0;
    hr = pSC->GetNumberOfCapabilities(&Count, &Size);
    checkForDShowError(hr, "DSCamera::dumpMediaTypes::GetNumberOfCapabilities");

    assert(Size == sizeof(VIDEO_STREAM_CONFIG_CAPS));
    bool bFormatFound = false;
    bool bCloseFormatFound = false;
    AM_MEDIA_TYPE *pmtConfig;
    AM_MEDIA_TYPE *pmtCloseConfig;
    vector<string> sImageFormats;
    VIDEOINFOHEADER * pvih;
    BITMAPINFOHEADER bih;
    for (int i = 0; i < Count; i++) {
        VIDEO_STREAM_CONFIG_CAPS scc;
        hr = pSC->GetStreamCaps(i, &pmtConfig, (BYTE*)&scc);
        checkForDShowError(hr, "DSCamera::dumpMediaTypes::GetStreamCaps");
        pvih = (VIDEOINFOHEADER*)(pmtConfig->pbFormat);
        bih = pvih->bmiHeader;
        double FrameRate = double(10000000L/pvih->AvgTimePerFrame);
        stringstream ss;
        ss << "  " << i << ": (" << bih.biWidth << "x" << bih.biHeight << "), " << 
                mediaSubtypeToString(pmtConfig->subtype) << 
                ", " << 10000000L/pvih->AvgTimePerFrame << " fps.";
        sImageFormats.push_back(ss.str());

        if (m_sPF != "") {
            if ( ((m_sPF == "MONO8" || m_sPF == "BY8_GBRG") && pmtConfig->subtype == MEDIASUBTYPE_Y800) ||
                 (m_sPF == "YUV422" && pmtConfig->subtype == MEDIASUBTYPE_UYVY) )
            {
                bFormatFound = true;
                break;
            } else {
                CoTaskMemFree((PVOID)pmtConfig->pbFormat);
                CoTaskMemFree(pmtConfig);
            }
        } else if (bih.biWidth == m_Size.x && bih.biHeight == m_Size.y && 
            ((bColor && 
                (pmtConfig->subtype == MEDIASUBTYPE_YUY2 ||
                 pmtConfig->subtype == MEDIASUBTYPE_UYVY ||
                 pmtConfig->subtype == MEDIASUBTYPE_Y411 ||
                 pmtConfig->subtype == MEDIASUBTYPE_Y41P ||
                 pmtConfig->subtype == MEDIASUBTYPE_YVYU)) ||
            (!bColor && pmtConfig->subtype == MEDIASUBTYPE_Y800)))
        {
            if (fabs(m_FrameRate-FrameRate) < 0.001) {
                bFormatFound = true;
                break;
            } else if (!bCloseFormatFound) {
                // The current format fits everything but the framerate.
                // Some cameras (Point Grey FireFly, for instance) don't report
                // all framerates, so we're going to try that as well.
                bCloseFormatFound = true;
                pmtCloseConfig = pmtConfig;
            } else {
                CoTaskMemFree((PVOID)pmtConfig->pbFormat);
                CoTaskMemFree(pmtConfig);
            }
        } else {
            CoTaskMemFree((PVOID)pmtConfig->pbFormat);
            CoTaskMemFree(pmtConfig);
        }
    }
    if (bFormatFound) {
        AVG_TRACE(Logger::CONFIG, "Camera image format: (" << bih.biWidth 
                << "x" << bih.biHeight << "), "
                << mediaSubtypeToString(pmtConfig->subtype) << ", " 
                << 10000000L/pvih->AvgTimePerFrame << " fps.");
        hr = pSC->SetFormat(pmtConfig);
        checkForDShowError(hr, "DSCamera::dumpMediaTypes::SetFormat");
        m_CameraPF = mediaSubtypeToPixelFormat(pmtConfig->subtype, m_sPF);
        CoTaskMemFree((PVOID)pmtConfig->pbFormat);
        CoTaskMemFree(pmtConfig);
        m_bCameraIsColor = bColor;
    } else {
        if (bCloseFormatFound) {
            // Set the framerate manually.
            pvih = (VIDEOINFOHEADER*)(pmtCloseConfig->pbFormat);
            bih = pvih->bmiHeader;
            pvih->AvgTimePerFrame = REFERENCE_TIME(10000000/m_FrameRate);
            AVG_TRACE(Logger::CONFIG, "Camera image format: (" << bih.biWidth 
                    << "x" << bih.biHeight << "), "
                    << mediaSubtypeToString(pmtCloseConfig->subtype) << ", " 
                    << 10000000L/pvih->AvgTimePerFrame << " fps.");
            hr = pSC->SetFormat(pmtCloseConfig);
            checkForDShowError(hr, "DSCamera::dumpMediaTypes::SetFormat");
            m_CameraPF = mediaSubtypeToPixelFormat(pmtCloseConfig->subtype, m_sPF);
            CoTaskMemFree((PVOID)pmtCloseConfig->pbFormat);
            CoTaskMemFree(pmtCloseConfig);
            m_bCameraIsColor = bColor;

            // Check if the framerate is what we wanted.
//            pSC->GetFormat(&p
            bFormatFound = true;
        } else if (bForce) {
            AVG_TRACE(Logger::WARNING, 
                "Possibly incomplete list of image formats supported by camera: ");
            for (unsigned i=0; i<sImageFormats.size(); i++) {
                AVG_TRACE(Logger::WARNING, sImageFormats[i]);
            }
            fatalError("Could not find suitable camera image format.");
        }
    }
	pSC->Release();
    return bFormatFound;
}

bool DSCamera::isCameraAvailable()
{
    return m_bCameraAvailable;
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

unsigned int DSCamera::getFeature(CameraFeature Feature) const
{
    long Prop = getDSFeatureID(Feature);
    long Val;
    long Flags;
    HRESULT hr;
    if (isDSFeatureCamControl(Feature)) {
        hr = m_pAMCameraControl->Get(Prop, &Val, &Flags);
    } else {
        hr = m_pCameraPropControl->Get(Prop, &Val, &Flags);
    }
    if (!SUCCEEDED(hr)) {
        AVG_TRACE(Logger::WARNING, "DSCamera::getFeature "
                +cameraFeatureToString(Feature)+" failed.");
        return 0;
    }
    return Val;
}

void DSCamera::setFeature(CameraFeature Feature, int Value, bool bIgnoreOldValue)
{
    long Prop = getDSFeatureID(Feature);
    if (!m_pCameraPropControl) {
        return;
    }
    long Flags;
    if (Value == -1) {
        Flags = KSPROPERTY_VIDEOPROCAMP_FLAGS_AUTO;
    } else {
        Flags = KSPROPERTY_VIDEOPROCAMP_FLAGS_MANUAL;
    }
    HRESULT hr;
    if (isDSFeatureCamControl(Feature)) {
        hr = m_pAMCameraControl->Set(Prop, Value, Flags);
    } else {
        hr = m_pCameraPropControl->Set(Prop, Value, Flags);
    }
    switch (hr) {
        case E_INVALIDARG:
            // TODO: Throw exception
            AVG_TRACE(Logger::ERROR, "DSCamera::setFeature(" 
                    << cameraFeatureToString(Feature) << ", " << Value << ") failed.");
            break;
        case E_PROP_ID_UNSUPPORTED:  
        case E_PROP_SET_UNSUPPORTED:
            AVG_TRACE(Logger::ERROR, "DSCamera::setFeature(" 
                << cameraFeatureToString(Feature) << ") failed: Feature not supported by camera.");
            break;
        default:
            checkForDShowError(hr, "DSCamera::setFeature()::Set value");
    }
}

void DSCamera::initGraphBuilder()
{
    HRESULT hr;
    // TODO: Check if the threading model is ok.
    hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    checkForDShowError(hr, "DSCamera::initGraphBuilder()::CoInitializeEx");

    // Create the filter graph
    hr = CoCreateInstance (CLSID_FilterGraph, NULL, CLSCTX_INPROC,
            IID_IGraphBuilder, (void **) &m_pGraph);
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
    IMoniker* pMoniker =NULL;
    ICreateDevEnum *pDevEnum =NULL;
    IEnumMoniker *pClassEnum = NULL;

    // Create the system device enumerator
    hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC,
                           IID_ICreateDevEnum, (void **) &pDevEnum);
    checkForDShowError(hr, "DSCamera::findCaptureDevice()::CreateDevEnum");

    // Create an enumerator for the video capture devices
    hr = pDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, 
            &pClassEnum, 0);
	checkForDShowError(hr, "DSCamera::findCaptureDevice()::CreateClassEnumerator");

	// If there are no enumerators for the requested type, then 
	// CreateClassEnumerator will succeed, but pClassEnum will be NULL.
	if (pClassEnum == NULL) {
        AVG_TRACE(Logger::WARNING, "No DirectShow capture device found. Disabling camera.");
        m_bCameraAvailable = false;
        *ppSrcFilter = 0;
        return;
    }

    vector<string> sDescriptions;
    vector<string> sFriendlyNames;
    vector<string> sDevicePaths;

    bool bFound = false;
    while (!bFound && pClassEnum->Next(1, &pMoniker, NULL) == S_OK) {
        IPropertyBag *pPropBag;
        hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag, (void**)(&pPropBag));
    	checkForDShowError(hr, "DSCamera::findCaptureDevice()::BindToStorage");

        string sDescription = getStringProp(pPropBag, L"Description");
        string sFriendlyName = getStringProp(pPropBag, L"FriendlyName");
        string sDevicePath = getStringProp(pPropBag, L"DevicePath");

        if (m_sDevice == sDescription  || m_sDevice == sFriendlyName || sDevicePath.find(m_sDevice) != -1 || m_sDevice == "") {
            bFound = true;
        } else {
            pMoniker->Release();
        }
        sDescriptions.push_back(sDescription);
        sFriendlyNames.push_back(sFriendlyName);
		sDevicePaths.push_back(sDevicePath);

        pPropBag->Release();
    }
    if (!bFound) {
        AVG_TRACE(Logger::WARNING, "Available cameras: ");
        for (unsigned i=0; i<sDescriptions.size(); i++) {
            char sz[256];
            AVG_TRACE(Logger::WARNING, "  "+string(_itoa(i, sz, 10))+
				": Description='"+sDescriptions[i]+"', Name: '"+sFriendlyNames[i]+"', Path: '"+sDevicePaths[i]+"'");
        }
        fatalError("DSCamera::findCaptureDevice(): Unable to access video capture device.");
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
    
    HRESULT hr = pGraph->Connect(pOut, pIn);
    checkForDShowError(hr, "DSCamera::ConnectFilters::Connect");
    pOut->Release();
    pIn->Release();
}

void DSCamera::getUnconnectedPin(IBaseFilter *pFilter, PIN_DIRECTION PinDir, IPin **ppPin)
{
    *ppPin = 0;
    IEnumPins *pEnum = 0;
    IPin *pPin = 0;
    HRESULT hr = pFilter->EnumPins(&pEnum);
    checkForDShowError(hr, "DSCamera::ConnectFilters::Connect");
    while (pEnum->Next(1, &pPin, NULL) == S_OK)
    {
        PIN_DIRECTION ThisPinDir;
        pPin->QueryDirection(&ThisPinDir);
        if (ThisPinDir == PinDir)
        {
            IPin *pTmp = 0;
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
    fatalError("DSCamera::getUnconnectedPin failed to find pin.");
}

void DSCamera::fatalError(const string & sMsg)
{
    AVG_TRACE(Logger::ERROR, sMsg);
    close();
    exit(1);
}

#pragma warning(disable : 4995)
void DSCamera::checkForDShowError(HRESULT hr, const string & sLocation)
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
    fatalError(sLocation+": "+szErr);
}


}
