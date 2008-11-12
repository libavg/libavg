// Copyright (C) 2008 Archimedes Solutions GmbH,
// Saarbrücker Str. 24b, Berlin, Germany
//
// This file contains proprietary source code and confidential
// information. Its contents may not be disclosed or distributed to
// third parties unless prior specific permission by Archimedes
// Solutions GmbH, Berlin, Germany is obtained in writing. This applies
// to copies made in any form and using any medium. It applies to
// partial as well as complete copies.

#ifndef _DSCamera_H_
#define _DSCamera_H_

#include "DSSampleQueue.h"

#include "../avgconfigwrapper.h"

#include "Camera.h"

#include "../graphics/Bitmap.h"
#include "../graphics/Pixel24.h"

#include "../base/Point.h"
#include "../base/Queue.h"
#include "../base/WorkerThread.h"

#include <string>
#define _WIN32_DCOM
#include <windows.h>
#include <dshow.h>
#include <Qedit.h>

namespace avg {

class DSCamera: public Camera {
public:
    DSCamera(std::string sDevice, IntPoint Size, std::string sPF,
            double FrameRate, bool bColor);
    virtual ~DSCamera();
    virtual void open();
    virtual void close();

    virtual IntPoint getImgSize();
    virtual BitmapPtr getImage(bool bWait);
    virtual bool isCameraAvailable();

    virtual const std::string& getDevice() const; 
    virtual const std::string& getDriverName() const; 
    virtual double getFrameRate() const;

    virtual unsigned int getFeature(CameraFeature Feature) const;
    virtual void setFeature(CameraFeature Feature, int Value, bool bIgnoreOldValue=false);

private:
    void initGraphBuilder();
    void findCaptureDevice(IBaseFilter ** ppSrcFilter);

    void setCaptureFormat();
    bool selectMediaType(bool bColor, bool bForce);
    void connectFilters(IGraphBuilder *pGraph, IBaseFilter *pSrc, 
            IBaseFilter *pDest);
    void getUnconnectedPin(IBaseFilter *pFilter, PIN_DIRECTION PinDir, 
            IPin **ppPin);
    void checkForDShowError(HRESULT hr, const std::string & sAppMsg);
    void fatalError(const std::string & sMsg);

    std::string m_sDevice;
    IntPoint m_Size;
    double m_FrameRate;
    bool m_bColor;
    bool m_bCameraIsColor;
    std::string m_sPF;
    PixelFormat m_CameraPF;

    bool m_bCameraAvailable;

    IGraphBuilder * m_pGraph;
    ICaptureGraphBuilder2 * m_pCapture;
    IMediaControl * m_pMediaControl;
    IBaseFilter * m_pSrcFilter;
    IBaseFilter * m_pGrabFilter;
	ISampleGrabber * m_pSampleGrabber;
    IAMVideoProcAmp * m_pCameraPropControl;
    IAMCameraControl * m_pAMCameraControl;

    DSSampleQueue * m_pSampleQueue;

    DWORD m_GraphRegisterID;
};

}

#endif
