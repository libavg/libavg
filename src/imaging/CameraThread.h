//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2006 Ulrich von Zadow
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

#ifndef _CameraThread_H_
#define _CameraThread_H_

#include "../avgconfig.h"
#undef PACKAGE_BUGREPORT
#undef PACKAGE_NAME
#undef PACKAGE_STRING
#undef PACKAGE_TARNAME
#undef PACKAGE_VERSION

#include "../graphics/Rect.h"
#include "../graphics/Bitmap.h"
#include "../graphics/Pixel24.h"

#include "../base/Queue.h"

#ifdef AVG_ENABLE_1394
#include <libraw1394/raw1394.h>
#include <libdc1394/dc1394_control.h>
#endif
#ifdef AVG_ENABLE_1394_2
#include <dc1394/control.h>
#endif

#include <string>

namespace avg {

typedef Queue<BitmapPtr> BitmapQueue;

typedef enum {
    STOP
} CameraCmd;
typedef Queue<CameraCmd> CameraCmdQueue;

class CameraThread {
public:
#ifdef AVG_ENABLE_1394
    CameraThread::CameraThread(BitmapQueue& BitmapQ, CameraCmdQueue& CmdQ, 
            std::string sDevice, int Mode, double FrameRate);
#else
    CameraThread::CameraThread(BitmapQueue& BitmapQ, CameraCmdQueue& CmdQ, 
            std::string sDevice, dc1394video_mode_t Mode, double FrameRate);
#endif
    void operator()();

private:
    void open();
    void close();
    bool captureImage();
    void checkMessages();

    bool m_bShouldStop;
    BitmapQueue& m_BitmapQ;
    CameraCmdQueue& m_CmdQ;

    std::string m_sDevice;
    double m_FrameRate;


#ifdef AVG_ENABLE_1394
    bool findCameraOnPort(int port, raw1394handle_t& FWHandle);

    dc1394_cameracapture m_Camera;
    raw1394handle_t m_FWHandle;
    dc1394_feature_set m_FeatureSet;
    int m_FrameRateConstant;  // libdc1394 constant for framerate.
    int m_Mode;               // libdc1394 constant for mode.
#else
    dc1394camera_t * m_pCamera;
    dc1394featureset_t m_FeatureSet;
    dc1394framerate_t m_FrameRateConstant; 
    dc1394video_mode_t m_Mode;            
#endif
    void checkDC1394Error(int Code, const std::string & sMsg);
    void fatalError(const std::string & sMsg);
    void dumpCameraInfo();

    bool m_bCameraAvailable;
};

}

#endif
