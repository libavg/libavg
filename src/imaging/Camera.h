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

#ifndef _Camera_H_
#define _Camera_H_

#include "../avgconfig.h"
#undef PACKAGE_BUGREPORT
#undef PACKAGE_NAME
#undef PACKAGE_STRING
#undef PACKAGE_TARNAME
#undef PACKAGE_VERSION
#if defined (AVG_ENABLE_1394) || defined (AVG_ENABLE_1394_2)
#include "CameraUtils.h"
#include "CameraThread.h"
#endif

#include "../graphics/Bitmap.h"
#include "../graphics/Pixel24.h"

#include "../base/Queue.h"

#include <boost/thread.hpp>

#include <string>
#include <map>

namespace avg {

class Camera
{
    public:
        Camera(std::string sDevice, double FrameRate, std::string sMode);
        virtual ~Camera();
        void open();
        void close();

        IntPoint getImgSize();
        BitmapPtr getImage();
        bool isCameraAvailabe();

        const std::string& getDevice() const; 
        double getFrameRate() const;
        const std::string& getMode() const;
        
        unsigned int getFeature(const std::string& sFeature) const;
        void setFeature(const std::string& sFeature, int Value);

    private:

        std::string m_sDevice;
        double m_FrameRate;
        std::string m_sMode;
        std::map<int, int> m_Features;

#if defined (AVG_ENABLE_1394) || defined (AVG_ENABLE_1394_2)
        BitmapQueue m_BitmapQ;
        CameraCmdQueue m_CmdQ;
#endif
        boost::thread* m_pThread;
};

typedef boost::shared_ptr<Camera> CameraPtr;

}

#endif

