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

    virtual int getFeature(CameraFeature Feature) const;
    virtual void setFeature(CameraFeature Feature, int Value, bool bIgnoreOldValue=false);
    virtual void setFeatureOneShot(CameraFeature Feature);
    virtual int getWhitebalanceU() const;
    virtual int getWhitebalanceV() const;
    virtual void setWhitebalance(int u, int v, bool bIgnoreOldValue=false);

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
